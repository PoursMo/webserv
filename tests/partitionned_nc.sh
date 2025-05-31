#!/bin/bash

(cat tests/http/_multipart1.http; sleep 5; cat tests/http/_multipart2.http) | nc -q 0 127.0.0.1 8000