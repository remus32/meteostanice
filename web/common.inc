<?php
require_once "vendor/autoload.php";

// Načíst .env soubor, obsahuje konfiguraci databaze
$dotenv = Dotenv\Dotenv::createImmutable(__DIR__);
$dotenv->load();

$dbh = new PDO($_ENV['DB_URL']);
$dbh->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
