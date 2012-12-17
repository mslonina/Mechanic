#!/bin/bash

for i in index install userguide; do
  php ${i}.tpl.php > ${i}.html
done
