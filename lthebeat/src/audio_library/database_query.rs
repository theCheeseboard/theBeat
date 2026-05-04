use gpui::{App, AppContext, AsyncApp, Entity};
use smol::stream::StreamExt;
use sqlx::sqlite::{SqliteArguments, SqliteRow};
use sqlx::{Arguments, Row, SqlitePool};
use std::ops::Range;

pub struct DatabaseQuery<RecordType>
where
    RecordType: DatabaseRecord,
{
    pool: SqlitePool,
    base_query: String,
    records: Box<[Option<Entity<RecordType>>]>,
    binds: SqliteArguments<'static>,
}

impl<RecordType> DatabaseQuery<RecordType>
where
    RecordType: DatabaseRecord + 'static,
{
    pub async fn new(
        pool: Option<SqlitePool>,
        base_query: String,
        binds: SqliteArguments<'static>,
    ) -> anyhow::Result<Self> {
        let pool = pool.ok_or(anyhow::anyhow!("No database pool"))?;
        let count_query_string = format!("SELECT COUNT(*) as count FROM ({base_query})");
        let count_query = sqlx::query_with(&count_query_string, binds.clone())
            .fetch_one(&pool.clone())
            .await?;

        let mut records = Vec::new();
        records.resize_with(count_query.get::<u32, _>("count") as usize, || {
            Default::default()
        });

        Ok(Self {
            pool,
            base_query,
            records: records.into_boxed_slice(),
            binds,
        })
    }

    pub fn get(&mut self, index: usize, cx: &mut App) -> Entity<RecordType> {
        let Some(record) = &self.records[index] else {
            // Query the database for 10 records
            let floored = index / 10 * 10;
            let until = self.records.len().min(floored + 10);
            for i in floored..until {
                let new_record = cx.new(|_| Default::default());
                self.records[i] = Some(new_record.clone());
            }

            let items: Vec<_> = self.records[floored..until]
                .iter()
                .map(|r| r.as_ref().unwrap().clone())
                .collect();
            let base_query = self.base_query.clone();
            let pool = self.pool.clone();
            let mut binds = self.binds.clone();

            cx.spawn(async move |cx: &mut AsyncApp| {
                let get_query_string = format!("SELECT * FROM ({base_query}) LIMIT ? OFFSET ?");
                binds.add(items.len() as u32).unwrap();
                binds.add(floored as u32).unwrap();
                let mut get_query = sqlx::query_with(&get_query_string, binds).fetch(&pool.clone());

                let mut i = 0_usize;
                while let Some(row) = get_query.next().await {
                    cx.update_entity(&items[i], |item, cx| {
                        item.read_from_row(row, &pool);
                        cx.notify();
                    });
                    i += 1;
                }
            })
            .detach();

            return self.records[index]
                .as_ref()
                .expect("Record was just created")
                .clone();
        };
        record.clone()
    }

    pub async fn populate_range(&mut self, range: Range<usize>, cx: &mut App) {
        let get_query_string = format!("SELECT * FROM ({}) LIMIT ? OFFSET ?", self.base_query);
        let pool = self.pool.clone();
        let mut binds = self.binds.clone();
        binds.add(range.end as u32 - range.start as u32).unwrap();
        binds.add(range.start as u32).unwrap();
        let mut get_query = sqlx::query_with(&get_query_string, binds).fetch(&pool);

        let mut i = range.start;
        while let Some(row) = get_query.next().await {
            let record = self.records[i].get_or_insert_with(|| cx.new(|_| Default::default()));
            cx.update_entity(record, |item, cx| {
                item.read_from_row(row, &pool);
                cx.notify();
            });
            i += 1;
        }
    }

    pub async fn populate_all(&mut self, cx: &mut App) {
        self.populate_range(0..self.count(), cx).await;
    }

    pub fn count(&self) -> usize {
        self.records.len()
    }

    pub fn iter<'this>(
        &'this mut self,
        cx: &'this mut App,
    ) -> DatabaseQueryIterator<'this, RecordType> {
        DatabaseQueryIterator {
            parent: self,
            cx,
            current: 0,
        }
    }
}

pub trait DatabaseRecord: Default {
    fn read_from_row(&mut self, row: Result<SqliteRow, sqlx::Error>, pool: &SqlitePool);
}

pub struct DatabaseQueryIterator<'parent, RecordType>
where
    RecordType: DatabaseRecord,
{
    parent: &'parent mut DatabaseQuery<RecordType>,
    cx: &'parent mut App,
    current: usize,
}

impl<'parent, RecordType> Iterator for DatabaseQueryIterator<'parent, RecordType>
where
    RecordType: DatabaseRecord + 'static,
{
    type Item = Entity<RecordType>;

    fn next(&mut self) -> Option<Self::Item> {
        if self.current == self.parent.count() {
            None
        } else {
            let next = self.parent.get(self.current, self.cx);
            self.current += 1;
            Some(next)
        }
    }

    fn count(self) -> usize
    where
        Self: Sized,
    {
        self.parent.count()
    }
}
