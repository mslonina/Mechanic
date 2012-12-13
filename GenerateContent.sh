#!/bin/bash

for i in index; do
  php ${i}.tpl.php > ${i}.html
done
