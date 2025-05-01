#!/usr/bin/bash
for i in $(find thesis/Figures -type f -name '*.ipe' -not -name 'edges_arrangements.ipe'); do
	FIGURE="${i/ipe/pdf}"
	iperender -pdf "$i" "$FIGURE"
	echo $FIGURE
done
scripts/generate_edge_crossings.py
