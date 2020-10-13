<?php
require_once "vendor/autoload.php";

date_default_timezone_set('Europe/Prague');

// Načíst .env soubor, obsahuje konfiguraci databaze
$dotenv = Dotenv\Dotenv::createImmutable(__DIR__);
$dotenv->load();

$dbh = new PDO($_ENV['DB_URL']);
$dbh->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);

// pět minut
define('WS_TIME_PERIOD', 5 * 60);
define('WS_WEEKDAYS', [
  'Po', 'Út', 'St', 'Čt', 'Pá', 'So', 'Ne'
]);
define('WS_CHART_Y_TICKS', 5);

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

/**
 * Převede časový interval ve formátu hh:mm:ss.ss do češtiny
 */
function pgduration2czech($str) {
  list($hours, $mins, $secs) = explode(':', $str);
  $hours = intval($hours);
  $mins = intval($mins);
  $secs = round(floatval($secs));

  $rv = '';
  if ($hours > 1) {
    $rv .= "$hours hodinami";
  } else if ($hours > 0) {
    $rv .= "hodinou";
  }

  // v tomhle řádu už stejně vteřiny nikoho nezajímaj
  if ($hours > 5) return $rv;

  if ($mins > 0) {
    if (strlen($rv)) $rv .= ' a ';
    $rv .= $mins > 1 ? "$mins minutami" : 'minutou';
  }

  if (strlen($rv) && $secs <= 10) return $rv;

  if (strlen($rv)) $rv .= ' a ';
  $rv .= $secs > 10 ? "$secs sekundami" : 'pár sekundami';

  return $rv;
}

function _series_min_max($arr, $field, $round, $chart_length, $invert_map = false) {
  $values = array_map(function($el) use($field) { return $el[$field]; }, $arr);

  $min = min($values);
  $max = max($values);

  if ($round > 0) {
    $min = floor($min / $round) * $round;
    $max = ceil($max / $round) * $round;
  }

  return [
    'values' => $values,
    'min' => $min,
    'max' => $max,
    'map' => function($v) use($chart_length, $invert_map, $min, $max) {
      // pomocná promměnná
      $x = $chart_length * ($v - $min) / ($max - $min);
      return $invert_map ? $chart_length - $x : $x;
    },
    'imap' => function($v) use($chart_length, $invert_map, $min, $max) {
      // pomocná promměnná
      $x = $invert_map ? 1 - ($v / $chart_length) : $v / $chart_length;
      return $min + ($x * ($max - $min));
    }
  ];
}

function chart_line($arr, $xfield, $yfield, $width, $height, $round, $color) {
  if (count($arr) < 3) return false;

  list('min' => $xmin, 'max' => $xmax, 'map' => $mapx) = _series_min_max($arr, $xfield, 0, $width);
  list('min' => $ymin, 'max' => $ymax, 'map' => $mapy) = _series_min_max($arr, $yfield, $round, $height, true);

  $rv = '';
  $path_d = '';
  foreach ($arr as $el) {
    $ox = $el[$xfield];
    $oy = $el[$yfield];
    $x = strval($mapx($ox));
    $y = strval($mapy($oy));

    $path_d .= strlen($path_d) ? ' L' : 'M';
    $path_d .= "$x $y";

    $rv .= "<circle cx=\"$x\" cy=\"$y\" r=\"3\" data-x=\"$ox\" data-y=\"$oy\" fill=\"$color\" />";
  }

  $rv .= "<path stroke=\"$color\" fill=\"none\" d=\"$path_d\">";

  return $rv;
}

function chart_period_axis($pts, $width) {
  $tick_len = 20;
  $attrs = "stroke=\"black\" stroke-width=\"3px\"";

  $tick_str = function ($xpos, $value, $disable_line) use ($attrs, $tick_len) {
    $time = wstime2unix($value);

    // Den v týdnu
    $text1 = WS_WEEKDAYS[intval(date('N', $time)) - 1];
    $text1_y = $tick_len + 20;
    // Čas
    $text2 = date('H:i', $time);
    $text2_y = $text1_y + 20;

    return ($disable_line ? "" : "<line x1=\"$xpos\" x2=\"$xpos\" y1=\"0\" y2=\"$tick_len\" $attrs />") .
      "<text text-anchor=\"middle\" x=\"$xpos\" y=\"$text1_y\">$text1</text>" .
      "<text text-anchor=\"middle\" x=\"$xpos\" y=\"$text2_y\">$text2</text>";
  };

  $rv = "<path d=\"M 0 $tick_len L 0 0 L $width 0 L $width $tick_len\" fill=\"none\" $attrs />";
  $rv .= $tick_str(0, $pts[0], true);
  $rv .= $tick_str($width, $pts[count($pts) - 1], true);

  return $rv;
}

function chart_double_y_axis($arr, $afield, $bfield, $around, $bround, $acolor, $bcolor, $a_str_fn, $b_str_fn, $height, $chart_width) {
  $tick_len = 8;
  $attrs = "stroke=\"black\" stroke-width=\"3px\"";
  $grid_attrs = "stroke=\"grey\" stroke-width=\"2px\"";

  list('min' => $amin, 'max' => $amax, 'imap' => $imapa) = _series_min_max($arr, $afield, $around, $height, true);
  list('min' => $bmin, 'max' => $bmax, 'imap' => $imapb) = _series_min_max($arr, $bfield, $bround, $height, true);

  $grid_str = function($ypos) use ($chart_width, $grid_attrs) {
    return "<line x1=\"0\" x2=\"$chart_width\" y1=\"$ypos\" y2=\"$ypos\" $grid_attrs />";
  };

  $tick_str = function($y, $disable_line = false) use ($attrs, $grid_str, $tick_len, $a_str_fn, $b_str_fn, $imapa, $imapb, $acolor, $bcolor) {
    $va = $imapa($y);
    $vb = $imapb($y);

    $texta = $a_str_fn($va);
    $textb = $b_str_fn($vb);

    $textx = -$tick_len - 2;

    $textay = $y - 3;
    $textby = $y + 15;

    $texty = $y + 5; // aby byl text spravne vertikalne zarovnany
    return ($disable_line ? "" : "<line x1=\"-$tick_len\" x2=\"0\" y1=\"$y\" y2=\"$y\" $attrs />" . $grid_str($y)) .
      "<text fill=\"$bcolor\" text-anchor=\"end\" x=\"$textx\" y=\"$textby\">$textb</text>" .
      "<text fill=\"$acolor\" text-anchor=\"end\" x=\"$textx\" y=\"$textay\">$texta</text>";
  };

  $rv = "<path d=\"M -$tick_len 0 L 0 0 L 0 $height L -$tick_len $height\" fill=\"none\" $attrs />";

  for ($i=0; $i < WS_CHART_Y_TICKS; $i++) {
    $rv .= $tick_str(($i / (WS_CHART_Y_TICKS - 1)) * $height);
  }

  return $rv;
}

function chart_right_y_axis($arr, $field, $round, $color, $str_fn, $height) {
  $tick_len = 8;
  $attrs = "stroke=\"black\" stroke-width=\"3px\"";

  list('min' => $min, 'max' => $max, 'imap' => $imap) = _series_min_max($arr, $field, $round, $height, true);

  $tick_str = function($y, $disable_line = false) use ($imap, $tick_len, $attrs, $color, $str_fn) {
    $v = $imap($y);

    $text = $str_fn($v);
    $text_pos = $tick_len + 2;

    $texty = $y + 5; // aby byl text spravne vertikalne zarovnany
    return ($disable_line ? "" : "<line x1=\"$tick_len\" x2=\"0\" y1=\"$y\" y2=\"$y\" $attrs />") .
      "<text fill=\"$color\" text-anchor=\"start\" x=\"$text_pos\" y=\"$texty\">$text</text>";
  };

  $rv = "<path d=\"M $tick_len 0 L 0 0 L 0 $height L $tick_len $height\" fill=\"none\" $attrs />";

  for ($i=0; $i < WS_CHART_Y_TICKS; $i++) {
    $rv .= $tick_str(($i / (WS_CHART_Y_TICKS - 1)) * $height);
  }

  return $rv;
}
