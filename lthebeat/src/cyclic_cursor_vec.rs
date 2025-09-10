#[derive(Clone)]
pub struct CyclicCursorVec<T> {
    pub vec: Vec<T>,
    current_index: usize,
    pub repeat_one: bool,
}

impl<T> CyclicCursorVec<T> {
    pub fn new() -> Self {
        Self {
            vec: Vec::new(),
            current_index: 0,
            repeat_one: false,
        }
    }

    pub fn new_from_vec(vec: Vec<T>) -> Self {
        Self {
            vec,
            current_index: 0,
            repeat_one: false,
        }
    }

    pub fn take_vec(self) -> Vec<T> {
        self.vec
    }

    pub fn is_empty(&self) -> bool {
        self.vec.is_empty()
    }

    pub fn next(&mut self) -> &T {
        if !self.repeat_one {
            self.current_index = (self.current_index + 1) % self.vec.len();
        }
        self.current()
    }

    pub fn prev(&mut self) -> &T {
        if !self.repeat_one {
            if self.current_index == 0 {
                self.current_index = self.vec.len() - 1;
            } else {
                self.current_index -= 1
            }
        }
        self.current()
    }

    pub fn set_current(&mut self, index: usize) {
        self.current_index = index;
    }

    pub fn peek(&self) -> &T {
        if self.repeat_one {
            &self.vec[self.current_index]
        } else {
            let next_index = (self.current_index + 1) % self.vec.len();
            &self.vec[next_index]
        }
    }

    pub fn current(&self) -> &T {
        &self.vec[self.current_index]
    }

    pub fn push(&mut self, item: T) {
        self.vec.push(item);
    }

    pub fn insert(&mut self, index: usize, item: T) {
        self.vec.insert(index, item);
        if self.current_index >= index {
            self.current_index += 1;
        }
    }

    pub fn retain<F>(&mut self, mut f: F)
    where
        F: FnMut(&T) -> bool,
    {
        let mut i = 0;
        while i < self.vec.len() {
            if f(&self.vec[i]) {
                i += 1;
            } else {
                self.vec.remove(i);
                if self.current_index >= i && self.current_index > 0 {
                    self.current_index -= 1;
                }
            }
        }
    }

    pub fn clear(&mut self) {
        self.vec.clear();
        self.current_index = 0;
    }

    pub fn len(&self) -> usize {
        self.vec.len()
    }

    pub fn repeat_one(&mut self, repeat: bool) {
        self.repeat_one = repeat;
    }

    pub fn set_vec(&mut self, vec: Vec<T>, current_index: usize) {
        self.vec = vec;
        self.current_index = current_index;
    }
}
