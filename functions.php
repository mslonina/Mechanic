<?php

function nav($active) {
  $nav = array(
    "index.html" => "Overview",
    "install.html" => "Installation",
    "userguide.html" => "User guide",
  );  

  $out = "<ul>";

  foreach ($nav as $key => $value) {
    if (basename($key, ".html") === $active) {
      $out .= '<li><a href="./'.$key.'" class="active">' .$value. '</a></li>'; 
    } else {
      $out .= '<li><a href="./'.$key.'">' .$value. '</a></li>'; 
    }
  }
  $out .= "</ul>";

  return $out;
}

function html_header() {
  $out = '
    <link rel="stylesheet" href="stylesheets/styles.css">
    <link rel="stylesheet" href="stylesheets/pygment_trac.css">
    <link rel="stylesheet" href="stylesheets/app.css">
    <link rel="stylesheet" href="stylesheets/prettify.css">
    <meta name="viewport" content="width=device-width, initial-scale=1, user-scalable=no">
    <script src="javascripts/jquery-1.8.3.min.js"></script>
    <!--[if lt IE 9]>
    <script src="//html5shiv.googlecode.com/svn/trunk/html5.js"></script>
    <![endif]-->
    ';

  return $out;
}

function html_footer() {
  $out = '
    <script src="javascripts/scale.fix.js"></script>
    <script src="javascripts/code.js"></script>
    <script src="prettify/prettify.js"></script>
    ';

  return $out;
}

?>
