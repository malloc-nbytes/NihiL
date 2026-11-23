struct Inner {
        z: i32,
}

struct Point {
        x: i32,
        y: i32,
        inner: Inner,
}

export proc _start(): i32
{
        let p: Point = Point {
                .x = 1,
                .y = 2,
                .inner = Inner {
                        .z = 3,
                },
        };

        let ar: Inner[2] = {Inner { .z = 1 }, Inner { .z = 2 }};

        p.inner.z = p.inner.z + (Inner { .z = 3 }).z;

        p.x = 3;

        let ptr: u8*;

        let ptr2: u8* = ptr + 1;

        return 0;
}
