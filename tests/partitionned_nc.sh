#!/bin/bash

(cat tests/http/part1.http; sleep 1; cat tests/http/part2.http) | nc -q 0 127.0.0.1 8000