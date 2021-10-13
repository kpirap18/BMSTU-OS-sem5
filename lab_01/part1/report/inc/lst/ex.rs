#[cfg(test)]
mod benchs {
    use super::*;

    #[bench]
    fn iterative10(b: &mut Bencher) {
        let s1 = generate_string_of_size(10);
        let s2 = generate_string_of_size(10);
        b.iter(|| algorithms::iterative(&s1, &s2));
    }
}
