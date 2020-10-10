<?php
require_once "vendor/autoload.php";

// Načíst .env soubor, obsahuje konfiguraci databaze
$dotenv = Dotenv\Dotenv::createImmutable(__DIR__);
$dotenv->load();

$dbh = new PDO($_ENV['DB_URL']);
$dbh->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);

// pět minut
define("WS_TIME_PERIOD", 5 * 60);

/**
 * Převede interní identifikátor časového intervalu do unixového času jeho počátku
 */
function wstime2unix($ws_time) {
    return $ws_time * WS_TIME_PERIOD;
}
/**
 * Převede unixový čas do interní identifikátoru časového intervalu
 */
function unix2wstime($unix_time) {
    return floor($unix_time / WS_TIME_PERIOD);
}
