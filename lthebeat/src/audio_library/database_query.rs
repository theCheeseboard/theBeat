use gpui::{App, AppContext, AsyncApp, Entity};
use smol::stream::StreamExt;
use sqlx::sqlite::{SqliteArguments, SqliteRow};
use sqlx::{Arguments, Row, SqlitePool};

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
        pool: SqlitePool,
        base_query: String,
        binds: SqliteArguments<'static>,
    ) -> anyhow::Result<Self> {
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
                        item.read_from_row(row);
                        cx.notify();
                    })
                    .unwrap();
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

    pub fn count(&self) -> usize {
        self.records.len()
    }
}

pub trait DatabaseRecord: Default {
    fn read_from_row(&mut self, row: Result<SqliteRow, sqlx::Error>);
}
