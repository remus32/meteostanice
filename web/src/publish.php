<?php
require_once "common.inc";

if ($_GET['key'] !== $_ENV['API_KEY']) die();

// https://stackoverflow.com/questions/7047870/issue-reading-http-request-body-from-a-json-post-in-php
$input_json = file_get_contents('php://input');
$input = json_decode($input_json, true);

$stmt = $dbh->prepare('INSERT INTO measurements("period", "temp", "hum", "pres") VALUES(?, ?, ?, ?)');

foreach ($input as $id => $measurement) {
 $stmt->execute(array(
    unix2wstime(time()) - ($id + 1), // Měření je za předchozí interval

    // Převod z fixed-point integerů do floatů
    $measurement['tsum'] / $measurement['n_bme'] / 100,
    $measurement['hsum'] / $measurement['n_bme'] / 1024,
    $measurement['psum'] / $measurement['n_bme'] / 256
  ));
}

// Kolik sekund už uběhlo od začátku aktuálního intervalu
$elapsed_period_time = time() - wstime2unix(unix2wstime(time()));
// Za kolik sekund začne další interval
$sync_time = ceil(WS_TIME_PERIOD - $elapsed_period_time);

echo("$sync_time\n");
