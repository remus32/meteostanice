<?php
require_once "common.inc";

/**
 * Spočítá průměr fieldů v poli objektů.
 */
function avgField($arr, $fieldName) {
  if (count($arr) === 0) return null;

  $sum = 0;
  foreach ($arr as $el) {
    $sum += $el[$fieldName];
  }
  return $sum / count($arr);
}

if ($_GET['key'] !== $_ENV['API_KEY']) die();

// https://stackoverflow.com/questions/7047870/issue-reading-http-request-body-from-a-json-post-in-php
$input_json = file_get_contents('php://input');
$input = json_decode($input_json, true);

$stmt = $dbh->prepare('INSERT INTO measurements("time", "temp", "hum", "pres") VALUES(?, ?, ?, ?)');
$stmt->execute(array(
  unix2wstime(time()) - 1, // Měření je za předchozí interval

  // Převod z fixed-point integerů do floatů
  avgField($input['bme'], 'temp') / 100,
  avgField($input['bme'], 'hum') / 1024,
  avgField($input['bme'], 'pres') / 256
));

// Kolik sekund už uběhlo od začátku aktuálního intervalu
$elapsed_period_time = time() - wstime2unix(unix2wstime(time()));
// Za kolik sekund začne další interval
$sync_time = ceil(WS_TIME_PERIOD - $elapsed_period_time);

echo("$sync_time\n");
