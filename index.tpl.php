<?php
  include_once "markdown.php";
?>
<!doctype html>
<html>
  <head>
    <meta charset="utf-8">
    <meta http-equiv="X-UA-Compatible" content="chrome=1">
    <title>Mechanic</title>

    <link rel="stylesheet" href="stylesheets/styles.css">
    <link rel="stylesheet" href="stylesheets/pygment_trac.css">
    <link rel="stylesheet" href="stylesheets/app.css">
    <link rel="stylesheet" href="stylesheets/prettify.css">
    <meta name="viewport" content="width=device-width, initial-scale=1, user-scalable=no">
    <script src="javascripts/jquery-1.8.3.min.js"></script>
    <!--[if lt IE 9]>
    <script src="//html5shiv.googlecode.com/svn/trunk/html5.js"></script>
    <![endif]-->
  </head>
  <body onload="styleCode()">
    <div class="wrapper">
      <header>
        <?php include("Header.html");?>
      <nav>
        <?php //include("Nav.html");?>
      </nav>
      </header>
      <section>
        <?php
          $file = "https://raw.github.com/mslonina/Mechanic/2.x/Overview.md";
          $content = Markdown(file_get_contents($file));
          print $content;
        ?>
      </section>

      <footer>
        <?php include("Footer.html");?>
      </footer>
    </div>
    <script src="javascripts/scale.fix.js"></script>
    <script src="javascripts/code.js"></script>
    <script src="prettify/prettify.js"></script>
    
  </body>
</html>
