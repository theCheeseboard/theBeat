use std::ops::{Add, Sub};
use std::time::Duration;
use libcdio_sys::lsn_t;

#[derive(Default, PartialEq, PartialOrd, Debug, Copy, Clone)]
pub struct Lsn(pub lsn_t);

impl Add<Lsn> for Lsn {
    type Output = Lsn;

    fn add(self, other: Lsn) -> Self::Output {
        Lsn(self.0 + other.0)
    }
}

impl Sub<Lsn> for Lsn {
    type Output = Lsn;

    fn sub(self, other: Lsn) -> Self::Output {
        Lsn(self.0 - other.0)
    }
}

impl From<Lsn> for i32 {
    fn from(lsn: Lsn) -> Self {
        lsn.0
    }
}

impl From<Lsn> for usize {
    fn from(lsn: Lsn) -> Self {
        lsn.0 as usize
    }
}

impl From<Lsn> for Duration {
    fn from(lsn: Lsn) -> Self {
        Duration::from_secs_f64(lsn.0 as f64 / 75.)
    }
}

impl From<Duration> for Lsn {
    fn from(duration: Duration) -> Self {
        Lsn((duration.as_secs_f64() * 75.).floor() as lsn_t)
    }
}