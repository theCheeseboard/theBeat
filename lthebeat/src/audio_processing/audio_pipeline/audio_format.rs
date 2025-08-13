use crate::audio_processing::sample::{Sample, SampleData};

#[derive(Clone, Copy)]
pub struct AudioFormat {
    pub channels: u16,
    pub sample_rate: u32,
    pub sample: SampleFormat,
}

#[derive(Clone, Copy, PartialEq)]
pub enum SampleFormat {
    Signed8,
    Unsigned8,
    Unsigned16,
    Signed16,
    Unsigned24,
    Signed24,
    Unsigned32,
    Signed32,
    Unsigned64,
    Signed64,
    Float32,
    Float64
}

impl From<Sample> for SampleFormat {
    fn from(value: Sample) -> Self {
        match value.data {
            SampleData::Signed8(_) => SampleFormat::Signed8,
            SampleData::Unsigned8(_) => SampleFormat::Unsigned8,
            SampleData::Unsigned16(_) => SampleFormat::Unsigned16,
            SampleData::Signed16(_) => SampleFormat::Signed16,
            SampleData::Unsigned24(_) => SampleFormat::Unsigned24,
            SampleData::Signed24(_) => SampleFormat::Signed24,
            SampleData::Unsigned32(_) => SampleFormat::Unsigned32,
            SampleData::Signed32(_) => SampleFormat::Signed32,
            SampleData::Unsigned64(_) => SampleFormat::Unsigned64,
            SampleData::Signed64(_) => SampleFormat::Signed64,
            SampleData::Float32(_) => SampleFormat::Float32,
            SampleData::Float64(_) => SampleFormat::Float64,
        }
    }
}