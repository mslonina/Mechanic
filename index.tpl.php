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
    <meta name="viewport" content="width=device-width, initial-scale=1, user-scalable=no">
    <!--[if lt IE 9]>
    <script src="//html5shiv.googlecode.com/svn/trunk/html5.js"></script>
    <![endif]-->
  </head>
  <body>
    <div class="wrapper">
      <header>
        <?php include("Header.html");?>
      </header>

      <section>
        <?php
          $file = "https://raw.github.com/mslonina/Mechanic/2.x/README.md";
          $content = Markdown(file_get_contents($file));
          print $content;
        ?>
      </section>

      <footer>
        <?php include("Footer.html");?>
      </footer>
    </div>
    <script src="javascripts/scale.fix.js"></script>
    
  </body>
</html>
