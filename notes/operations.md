# Operations

- arithmetic
- cumulative arithmetic
- summary statistics: count, mean, standard deviation, moments
- count, remove, or replace NaN or other special/invalid values
- clip
- dot product
- linear combination, many columns
- filter (various predicates?)
- argsort + reorder = sort

## Relational-style

- joins: inner, left/right, outer

## Time series

- cumulative min, max
- windowed moving average
- exponential moving average
- running Z score (demean / de-variance)

## Finance

Many Finance data operations are on tables with a schema of the form `(time, id..., data...)`, where `time` is a timestamp, `id...` is one or more columns identifying an instrument, and `data...` are the data columns.  The timestamps may be fully asynchronous ("tick data") or bucketed / intervalized to a preset clock, _e.g._ every 10 minutes.

- fill forward missing data
- time bucketing operations on ticks: find, in each time bucket, the high, low, last, mean, standard deviation
