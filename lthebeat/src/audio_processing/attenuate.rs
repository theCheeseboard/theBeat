use crate::audio_processing::sample::{SampleFrom, SampleInto};

pub trait Attenuate {
    fn attenuated(self, factor: f64) -> Self;
}

impl<T> Attenuate for T
where
    T: SampleInto<f64> + SampleFrom<f64>,
{
    fn attenuated(self, factor: f64) -> Self {
        T::sample_from(self.sample_into().attenuated(factor))
    }
}

impl Attenuate for f64 {
    fn attenuated(self, factor: f64) -> Self {
        <f64>::clamp(self * factor, -1.0, 1.0)
    }
}
