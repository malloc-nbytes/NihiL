struct Point {
        x: i32,
        y: i32,
}

proc sum(a: i32, b: i32): i32
{
        let s: i32 = a + b;
        return s;
}

export proc _start(): void
{
        let u: u8 = 3;

        if 1 {
                let x: u8* = "hello" + 1;
        }
}
