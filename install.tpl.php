<?php
  include_once "functions.php";
  include_once "markdown.php";
?>
<!doctype html>
<html>
  <head>
    <meta charset="utf-8">
    <meta http-equiv="X-UA-Compatible" content="chrome=1">
    <title>Mechanic :: Installation</title>
    <?php print html_header();?>
  </head>
  <body>
    <?php print ribbon();?>
    <div class="wrapper">
      <header>
        <?php include("Header.html");?>
        <?php print nav("install");?>
      </header>
      <nav>
        <?php //print nav("install");?>
      </nav>

      <section>
        <?php
          $file = "https://raw.github.com/mslonina/Mechanic/2.x/INSTALL.md";
          $content = Markdown(file_get_contents($file));
          print $content;
        ?>
      </section>

      <footer>
        <?php include("Footer.html");?>
      </footer>
    </div>
    <?php print html_footer();?>
    
  </body>
</html>
