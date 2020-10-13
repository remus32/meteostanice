#!/bin/bash
rsync -rvh dist/* vendor src/*.{inc,php,svg} steve.remus32.cz:/var/www/ws
