#[derive(Clone)]
pub struct CyclicCursorVec<T> {
    vec: Vec<T>,
    current_index: usize,
}

impl<T> CyclicCursorVec<T> {
    pub fn new() -> Self {
        Self {
            vec: Vec::new(),
            current_index: 0
        }
    }
    
    pub fn new_from_vec(vec: Vec<T>) -> Self {
        Self {
            vec,
            current_index: 0
        }
    }
    
    pub fn take_vec(self) -> Vec<T> {
        self.vec
    }
    
    pub fn is_empty(&self) -> bool {
        self.vec.is_empty()
    }
    
    pub fn next(&mut self) -> &T {
        self.current_index = (self.current_index + 1) % self.vec.len();
        self.current()
    }
    
    pub fn prev(&mut self) -> &T {
        if self.current_index == 0 {
            self.current_index = self.vec.len() - 1;
        } else {
            self.current_index -= 1
        }
        self.current()
    }
    
    pub fn peek(&self) -> &T {
        let next_index = (self.current_index + 1) % self.vec.len();
        &self.vec[next_index]
    }
    
    pub fn current(&self) -> &T {
        &self.vec[self.current_index]
    }
    
    pub fn push(&mut self, item: T) {
        self.vec.push(item);
    }
    
    pub fn remove(&mut self, index: usize) -> T {
        self.vec.remove(index);
        todo!("Ensure current_index is not out of bounds")
    }
    
    pub fn insert(&mut self, index: usize, item: T) {
        self.vec.insert(index, item);
        todo!("Ensure current_index is not out of bounds")
    }
    
    pub fn len(&self) -> usize {
        self.vec.len()
    }
}