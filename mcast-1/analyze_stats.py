#!/usr/bin/env python3
import sys, os
import pandas as pd
from hdrh.histogram import HdrHistogram
# from pyhdrh import HdrHistogram

if len(sys.argv) > 2:
    print(f"Usage: {sys.argv[0]} [csv_file]")
    sys.exit(1)

csv_file = sys.argv[1] if len(sys.argv) == 2 else "latency.csv"
if not os.path.isfile(csv_file):
    print(f"Error: File '{csv_file}' does not exist.")
    sys.exit(1)

df = pd.read_csv(csv_file)

if not {'seq', 'latency_ns'}.issubset(df.columns):
    print("CSV must have columns: seq,latency_ns (header must match exactly)")
    print(f"Found columns: {list(df.columns)}")
    sys.exit(2)

lat = df['latency_ns']



h = HdrHistogram(1, int(lat.max()), 3)
for v in lat:
    h.record_value(int(v))

print("\nHDR Histogram percentile distribution table:")
HdrHistogram.dump(h.encode())

print("\nHDR Histogram summary:")
print(f"min: {h.get_min_value()}")
print(f"mean: {h.get_mean_value():.1f}")
print(f"median: {h.get_value_at_percentile(50)}")
print(f"stddev: {h.get_stddev():.1f}")
print(f"99th percentile: {h.get_value_at_percentile(99)}")
print(f"99.9th percentile: {h.get_value_at_percentile(99.9)}")
print(f"max: {h.get_max_value()}")


print("\npandas stat summary:")
print(f"min: {lat.min()}")
print(f"mean: {lat.mean():.1f}")
print(f"median: {lat.median()}")
print(f"99th percentile: {lat.quantile(0.99):.1f}")
print(f"99.9th percentile: {lat.quantile(0.999):.1f}")
print(f"max: {lat.max()}")