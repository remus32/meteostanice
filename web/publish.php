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
$inputJSON = file_get_contents('php://input');
$input = json_decode($inputJSON, true);

$stmt = $dbh->prepare('INSERT INTO measurements("temp", "hum", "pres") VALUES(?, ?, ?)');
$stmt->execute(array(
  avgField($input['bme'], 'temp'),
  avgField($input['bme'], 'hum'),
  avgField($input['bme'], 'pres')
));
