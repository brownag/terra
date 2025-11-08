# Remove holes from polygons

Remove the holes in SpatVector polygons. If `inverse=TRUE` the holes are
returned (as polygons).

## Usage

``` r
# S4 method for class 'SpatVector'
fillHoles(x, inverse=FALSE)
```

## Arguments

- x:

  SpatVector

- inverse:

  logical. If `TRUE` the holes are returned as polygons

## Value

SpatVector

## See also

[`snap`](https://rspatial.github.io/terra/reference/topology.md),
[`gaps`](https://rspatial.github.io/terra/reference/gaps.md)

## Examples

``` r
x <- rbind(c(-10,0), c(140,60), c(160,0), c(140,-55))
hole <- rbind(c(80,0), c(105,13), c(120,2), c(105,-13))

z <- rbind(cbind(object=1, part=1, x, hole=0), 
       cbind(object=1, part=1, hole, hole=1))
colnames(z)[3:4] <- c('x', 'y')
p <- vect(z, "polygons", atts=data.frame(id=1))
p
#>  class       : SpatVector 
#>  geometry    : polygons 
#>  dimensions  : 1, 1  (geometries, attributes)
#>  extent      : -10, 160, -55, 60  (xmin, xmax, ymin, ymax)
#>  coord. ref. :  
#>  names       :    id
#>  type        : <num>
#>  values      :     1

f <- fillHoles(p)
g <- fillHoles(p, inverse=TRUE)

plot(p, lwd=16, border="gray", col="light yellow")
polys(f, border="blue", lwd=3, density=4, col="orange")
polys(g, col="white", lwd=3)
```
