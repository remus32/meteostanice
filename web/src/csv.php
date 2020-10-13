<?php
require_once "common.inc";

$del = ',';
$nl = "\n";

function print_arr($arr) {
  global $del, $nl;

  $size = count($arr);
  for ($i=0; $i < $size; $i++) {
    if ($i > 0) echo($del);
    echo($arr[$i]);
  }
  echo($nl);
}

$period_length = WS_TIME_PERIOD;
$data = $dbh->query("SELECT period * $period_length, (to_timestamp(period * $period_length) at time zone 'utc')::timestamp, temp, hum, pres FROM measurements ORDER BY period", PDO::FETCH_NUM);

header('Content-Type: text/csv');
header('Content-Disposition: attachment; filename="meteostanice.csv"');

print_arr(['CAS_UNIX', 'CAS_UTC', 'TEPLOTA', 'REL_VLHKOST', 'TLAK']);

foreach ($data as $row) {
  print_arr($row);
}
