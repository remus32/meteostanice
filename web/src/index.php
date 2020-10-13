<?php
require_once "common.inc";

$dbh->beginTransaction();
$dbh->exec('SET TRANSACTION ISOLATION LEVEL REPEATABLE READ READ ONLY');
list('period' => $last_period, 'time' => $last_updated, 'update' => $time_to_update) =
  $dbh->query('SELECT MAX(period) AS period, NOW() - MAX(time) AS time, EXTRACT(epoch FROM \'5 minutes 10 seconds\'::interval - (NOW() - MAX(time))) AS update FROM measurements')->fetchAll()[0];

$time_to_update = $time_to_update < 10 ? 5.1 * 60 : $time_to_update;

$current_data_stmt = $dbh->prepare('SELECT * FROM measurements WHERE period > ? ORDER BY period');
$current_data_stmt->execute(array($last_period - (12 * 24 * 3)));
$current_data = $current_data_stmt->fetchAll();

?>
<!DOCTYPE html>
<html lang="cs">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Meteostanice</title>

  <link rel="stylesheet" href="index.css">
  <script>setTimeout(function () { location.reload(); }, <?=1000 * $time_to_update?>)</script>
</head>
<body>
  <header>
    <div class="width-wrapper">
      Meteostanice
      <img class="logo" src="icon.svg" alt="Mráček">
    </div>
  </header>
  <main>
    <div class="width-wrapper">
      <span class="last-update">
        Poslední aktualizace před <?= pgduration2czech($last_updated) ?>
      </span>

      <svg viewBox="0 0 900 600">
        <g transform="translate(120 30)">
          <g transform="translate(0 500)">
            <?= chart_period_axis(array_map(function ($el) { return $el['period']; }, $current_data), 700) ?>
          </g>
          <?= chart_double_y_axis(
            $current_data,
            'temp', 'hum',
            5, 10,
            'red', 'green',
            function ($va) { $va = round($va); return "$va °C"; },
            function ($vb) { $vb = round($vb); return "$vb %rH"; },
            500,
            45, 700
          ) ?>
          <g transform="translate(700 0)">
            <?= chart_right_y_axis(
              $current_data,
              'pres',
              100,
              'blue',
              function ($v) { $v = round($v) / 1000; return "$v hPa"; },
              500
            ) ?>
          </g>
          <g>
            <?= chart_line($current_data, 'period', 'temp', 700, 500, 5, 'red') ?>
          </g>
          <g>
            <?= chart_line($current_data, 'period', 'hum', 700, 500, 10, 'green') ?>
          </g>
          <g>
            <?= chart_line($current_data, 'period', 'pres', 700, 500, 100, 'blue') ?>
          </g>
        </g>
      </svg>

      <div>

      </div>
    </div>
  </main>
  <footer>

  </footer>
</body>
</html>