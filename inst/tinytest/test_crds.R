library(terra)
library(tinytest)
m <- matrix(c(0,1,0,0,0,1), ncol=2)
v <- vect(m, type="polygons")
x <- as.vector(crds(v))

expect_equal(x, c(0,1,0,0,0,0,1,0))

m <- matrix(c(0,1,0,NaN,0,1), ncol=2)
r <- rast(m)
expect_equal(nrow(crds(r)), 5)
expect_equal(nrow(crds(r, na.rm=FALSE)), 6)
expect_equal(nrow(crds(rast(r))), 6)

