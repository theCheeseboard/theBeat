use crate::audio_processing::audio_metadata::AudioMetadata;
use cpal::U24;
use intx::I24;
use rand::random;
use std::fmt::{Debug, Formatter};
use std::time::Duration;
use gpui::Entity;
use crate::play_queue::media_item::MediaItem;

#[derive(Clone, Debug)]
pub struct Sample {
    pub sample_rate: u32,
    pub channels: u16,
    pub meta: AudioMetadata,

    /// A random number to keep track of this sample as it moves through the audio pipeline
    pub sample_id: u64,
    pub data: SampleData,

    pub elapsed_since_start: Option<Duration>,
    pub associated_track: Option<Entity<MediaItem>>
}

#[derive(Clone)]
pub enum SampleData {
    Empty,
    Signed8(Vec<i8>),
    Unsigned8(Vec<u8>),
    Unsigned16(Vec<u16>),
    Signed16(Vec<i16>),
    Unsigned24(Vec<U24>),
    Signed24(Vec<I24>),
    Unsigned32(Vec<u32>),
    Signed32(Vec<i32>),
    Unsigned64(Vec<u64>),
    Signed64(Vec<i64>),
    Float32(Vec<f32>),
    Float64(Vec<f64>),
}

impl Debug for SampleData {
    fn fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
        match self {
            SampleData::Empty => {
                write!(f, "empty")
            }
            SampleData::Signed8(v) => {
                write!(f, "Signed8, len: {}", v.len())
            }
            SampleData::Unsigned8(v) => {
                write!(f, "Unsigned8, len: {}", v.len())
            }
            SampleData::Unsigned16(v) => {
                write!(f, "Unsigned16, len: {}", v.len())
            }
            SampleData::Signed16(v) => {
                write!(f, "Signed16, len: {}", v.len())
            }
            SampleData::Unsigned24(v) => {
                write!(f, "Unsigned24, len: {}", v.len())
            }
            SampleData::Signed24(v) => {
                write!(f, "Signed24, len: {}", v.len())
            }
            SampleData::Unsigned32(v) => {
                write!(f, "Unsigned32, len: {}", v.len())
            }
            SampleData::Signed32(v) => {
                write!(f, "Signed32, len: {}", v.len())
            }
            SampleData::Unsigned64(v) => {
                write!(f, "Unsigned64, len: {}", v.len())
            }
            SampleData::Signed64(v) => {
                write!(f, "Signed64, len: {}", v.len())
            }
            SampleData::Float32(v) => {
                write!(f, "Float32, len: {}", v.len())
            }
            SampleData::Float64(v) => {
                write!(f, "Float64, len: {}", v.len())
            }
        }
    }
}

impl Sample {
    pub fn new(
        sample_rate: u32,
        channels: u16,
        meta: AudioMetadata,
        sample_id: Option<u64>,
        elapsed: Option<Duration>,
        data: SampleData,
        associated_track: Option<Entity<MediaItem>>
    ) -> Self {
        Sample {
            sample_rate,
            channels,
            meta,
            sample_id: sample_id.unwrap_or_else(|| random()),
            elapsed_since_start: elapsed,
            data,
            associated_track,
        }
    }

    pub fn len(&self) -> usize {
        match &self.data {
            SampleData::Empty => 0,
            SampleData::Signed8(vec) => vec.len(),
            SampleData::Unsigned8(vec) => vec.len(),
            SampleData::Unsigned16(vec) => vec.len(),
            SampleData::Signed16(vec) => vec.len(),
            SampleData::Unsigned24(vec) => vec.len(),
            SampleData::Signed24(vec) => vec.len(),
            SampleData::Unsigned32(vec) => vec.len(),
            SampleData::Signed32(vec) => vec.len(),
            SampleData::Unsigned64(vec) => vec.len(),
            SampleData::Signed64(vec) => vec.len(),
            SampleData::Float32(vec) => vec.len(),
            SampleData::Float64(vec) => vec.len(),
        }
    }

    pub fn is_empty(&self) -> bool {
        self.len() == 0
    }

    pub fn into_f32(self) -> Vec<f64> {
        match self.data {
            SampleData::Empty => Vec::new(),
            SampleData::Signed8(v) => v.into_iter().map(|s| s.sample_into()).collect(),
            SampleData::Unsigned8(v) => v.into_iter().map(|s| s.sample_into()).collect(),
            SampleData::Unsigned16(v) => v.into_iter().map(|s| s.sample_into()).collect(),
            SampleData::Signed16(v) => v.into_iter().map(|s| s.sample_into()).collect(),
            SampleData::Unsigned24(v) => todo!(),
            SampleData::Signed24(v) => todo!(),
            SampleData::Unsigned32(v) => v.into_iter().map(|s| s.sample_into()).collect(),
            SampleData::Signed32(v) => v.into_iter().map(|s| s.sample_into()).collect(),
            SampleData::Unsigned64(v) => todo!(),
            SampleData::Signed64(v) => todo!(),
            SampleData::Float32(v) => v.into_iter().map(|s| s.sample_into()).collect(),
            SampleData::Float64(v) => v.into_iter().collect(),
        }
    }

    pub fn convert_from_f64(self, f: Vec<f64>) -> Self {
        let new_sample_data = match self.data {
            SampleData::Empty => SampleData::Empty,
            SampleData::Signed8(_) => {
                SampleData::Signed8(f.into_iter().map(i8::sample_from).collect())
            }
            SampleData::Unsigned8(_) => {
                SampleData::Unsigned8(f.into_iter().map(u8::sample_from).collect())
            }
            SampleData::Unsigned16(_) => {
                SampleData::Unsigned16(f.into_iter().map(u16::sample_from).collect())
            }
            SampleData::Signed16(_) => {
                SampleData::Signed16(f.into_iter().map(i16::sample_from).collect())
            }
            SampleData::Unsigned24(_) => todo!(),
            SampleData::Signed24(_) => todo!(),
            SampleData::Unsigned32(_) => {
                SampleData::Unsigned32(f.into_iter().map(u32::sample_from).collect())
            }
            SampleData::Signed32(_) => {
                SampleData::Signed32(f.into_iter().map(i32::sample_from).collect())
            }
            SampleData::Unsigned64(_) => todo!(),
            SampleData::Signed64(_) => todo!(),
            SampleData::Float32(_) => {
                SampleData::Float32(f.into_iter().map(f32::sample_from).collect())
            }
            SampleData::Float64(_) => SampleData::Float64(f),
        };

        Self {
            channels: self.channels,
            sample_rate: self.sample_rate,
            meta: self.meta,
            sample_id: self.sample_id,
            elapsed_since_start: self.elapsed_since_start,
            data: new_sample_data,
            associated_track: self.associated_track,       
        }
    }
}

pub trait UnwrapSample<T>: Debug {
    fn unwrap(&self) -> &T;
}

macro_rules! unwrap_impl {
    ($t:ty, $m:path) => {
        impl UnwrapSample<Vec<$t>> for Sample {
            fn unwrap(&self) -> &Vec<$t> {
                match &self.data {
                    $m(v) => v,
                    _ => panic!(
                        "invalid sample format during unwrap. expected: {}, got: {:?}",
                        stringify!($m),
                        self
                    ),
                }
            }
        }
    };
}

unwrap_impl!(f64, SampleData::Float64);
unwrap_impl!(f32, SampleData::Float32);
unwrap_impl!(u64, SampleData::Unsigned64);
unwrap_impl!(u32, SampleData::Unsigned32);
unwrap_impl!(U24, SampleData::Unsigned24);
unwrap_impl!(u16, SampleData::Unsigned16);
unwrap_impl!(u8, SampleData::Unsigned8);
unwrap_impl!(i64, SampleData::Signed64);
unwrap_impl!(i32, SampleData::Signed32);
unwrap_impl!(I24, SampleData::Signed24);
unwrap_impl!(i16, SampleData::Signed16);
unwrap_impl!(i8, SampleData::Signed8);

trait SampleInto<T> {
    fn sample_into(self) -> T;
}

impl SampleInto<f64> for f32 {
    fn sample_into(self) -> f64 {
        self as f64
    }
}

macro_rules! f64_to {
    ($t:ty, $max_type:ty, $offset:expr) => {
        impl SampleInto<f64> for $t {
            fn sample_into(self) -> f64 {
                f64::from(self) / (f64::from(<$max_type>::MAX)) + $offset
            }
        }
    };
}

f64_to!(u32, i32, -1.0);
f64_to!(u16, i16, -1.0);
f64_to!(u8, i8, -1.0);
f64_to!(i32, i32, 0.0);
f64_to!(i16, i16, 0.0);
f64_to!(i8, i8, 0.0);

trait SampleFrom<T> {
    fn sample_from(value: T) -> Self;
}

impl SampleFrom<f64> for f32 {
    fn sample_from(value: f64) -> Self {
        value as f32
    }
}

macro_rules! f64_from {
    ($t:ty, $max_type:ty, $offset:expr) => {
        impl SampleFrom<f64> for $t {
            fn sample_from(value: f64) -> $t {
                ((value - $offset) * f64::from(<$max_type>::MAX)) as $t
            }
        }
    };
}

f64_from!(u32, i32, -1.0);
f64_from!(u16, i16, -1.0);
f64_from!(u8, i8, -1.0);
f64_from!(i32, i32, 0.0);
f64_from!(i16, i16, 0.0);
f64_from!(i8, i8, 0.0);
