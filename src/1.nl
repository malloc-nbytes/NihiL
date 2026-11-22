extern proc malloc(b: u32): void*;

struct Point {
        x: i32,
        y: i32,
}

proc f(p: Point): Point
{
}

export proc _start(): i32
{
        let p: Point = Point {
                .x = 1,
                .y = 2,
        };

        return 0;
}
