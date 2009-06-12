<?php include_once('utils.php'); ?>

<!DOCTYPE html
PUBLIC '-//W3C//DTD XHTML 1.0 Transitional//EN'
'http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd'>

<html xmlns='http://www.w3.org/1999/xhtml'>

<head>
	<title>Lung Retriever</title>
	<link rel='stylesheet' type='text/css' href='style.css' />
	<script language='javascript' src='script.js'></script>
        <meta http-equiv="Content-Type" content="text/html; charset=utf-8">
</script>
</head>

<body>
<div id='main'>
	<div id='header'>
		<?php include('header.php'); ?>
	</div>

	<div id='uploader'>
		<div class='center'>
		<div class='center-help'>
			<form action='index.php' method='post' enctype='multipart/form-data'>
						<input type='hidden' name='MAX_FILE_SIZE' value='3000000' /><!-- 3MB -->
						<label for="file">Escolha a imagem de exemplo da busca:</label>
						<input name='file' type='file' />
						<span><input type="radio" name="lado" value="left" checked="true" />Esquerdo</span>
						<span><input type="radio" name="lado" value="right" />Direito</span>
						<input type='submit' name='submit' value='pesquisa' />
						<br />
						<font size="-1">Tipos de arquivos permitidos: png, dcm e jpg.</font>
			</form>
		</div>
		</div>	
	</div>
	
	<?php
		$display_search = false;
		if($_POST['submit'] == 'aval') {
			$id = intval($_POST['imageId']);
			$ct = getCTScan($id);
			$lungId = getLungId($id, $_POST['lado']);
			for ($i = 0; $i < 12; $i++) {
				if ($_POST['similar'.$i] != '') {
					$values = explode(',', $_POST['similar'.$i]);
					adjustNeuralNetwork($lungId, $values[0], $values[1]);
				}
			}
			$display_search = true;
		}
		
		if ($_FILES["file"]["size"] < 3145728 && $_POST['submit'] == 'pesquisa') { // 3MB
			if ($_FILES["file"]["error"] > 0) {
		   	$display_search = false;
		   }
		   else {
		   	$ct = addCTScan($_FILES["file"]);
		   	$display_search = true;
		   }
		}

		if($display_search) {
	?>
	
	<hr align="center" />

	<div id='retrieval'>
		<div id='main-image'>
			<img src="ct/png/<?php echo $ct['filename']; ?>.png" width="256" height="256" />
			<div id='segmented-lungs'>
				<div id='left-lung'>
					<img src="lungs/png/<?php echo $ct['filename']; ?>_left.png" width="120" height="120" <?php echo ($_POST['lado'] == 'left')?'class="selected"':'' ?> />
					<select name="left" size="1" class='evaluation-mode off'>
						<option value="5">5 - Perfeita</option>
						<option value="4">4 - Detalhes errados</option>
						<option value="3">3 - Buracos</option>
						<option value="2">2 - Faltam partes importantes</option>
						<option value="1">1 - Tudo errado</option>
						<option value="0" selected='true'>Não opinar</option>
					</select>
				</div>
				<div id='right-lung'>
					<img src="lungs/png/<?php echo $ct['filename']; ?>_right.png" width="120" height="120" <?php echo ($_POST['lado'] == 'left')?'':'class="selected"' ?>/>
					<select name="right" size="1" class='evaluation-mode off'>
						<option value="5">5 - Perfeita</option>
						<option value="4">4 - Detalhes errados</option>
						<option value="3">3 - Buracos</option>
						<option value="2">2 - Faltam partes importantes</option>
						<option value="1">1 - Tudo errado</option>
						<option value="0" selected='true'>Não opinar</option>
					</select>
				</div>
			</div>
		</div>
		
		<div id='retrieved-images'>
			
			<?php
				$similar = getSimilarImages($ct['id'], 12, $_POST['lado'] == 'left');
				for($i = 0; $i < 12 && $i < count($similar); $i++) {
			?>
				<div class='retrieved-image'>
					<img src="lungs/png/<?php echo $similar[$i]['name'].'_'.$similar[$i]['position']; ?>.png" width="120" height="120" />
					<div class='RWbuttons evaluation-mode off'>
						<?php echo '<!--'.$similar[$i]['id'].'-->' ?>
						<div class='button right off' onClick='buttonClick(this);'><!-- --></div>
						<div class='button wrong off' onClick='buttonClick(this);'><!-- --></div>
					</div>
					<!--<?php echo $similar[$i]['value'] ?>-->
				</div>
			<?php
				}
			?>
		</div>
	</div>
	
	<div class='break'><!--  --></div>	
	
	<div id="evaluation">
		<div class='center'>
		<div class='center-help'>
			<form action="index.php" method="post" enctype="multipart/form-data">
				<button type='button' onClick='toggleEvaluationMode();'>Modo de avaliação</button>
				<button type='submit' name='submit' value='aval' onClick='aval(this);' class='evaluation-mode off'>Enviar avaliação</button>
				<input type="hidden" name="imageId" value="<?php echo $ct['id'] ?>">
				<input type="hidden" name="lado" value="<?php echo $_POST['lado'] ?>">
				<?php
					for($i = 0; $i < 12 && $i < count($similar); $i++) {
						echo '<input type="hidden" name="similar'.$i.'" id="similar'.$i.'" value="">'."\n\t\t\t\t";
					}
				?>
			</form>
		</div>
		</div>
	</div>
	
	<?php
		}
	?>
	
	<?php include('footer.php'); ?>
</div>
</body>

</html>
