<?php

function createRandomName($lenght) {
    $chars = "abcdefghijklmnopqrstuvwxyz0123456789";
    srand((double)microtime()*1000000);
    
    $random = '';
    for ($i = 0; $i <= $lenght; $i++) {
        $num = rand() % strlen($chars);
        $tmp = substr($chars, $num, 1);
        $random .= $tmp;
    }
    return $random;
}

function getCharacteristics($path){
	$fp = fopen($path, "r");
	$array = array();
	
	while($line = fgets($fp)) {
		$pos = strpos($line, ':');
		$key = substr($line, 0, $pos);
		$value = substr($line, $pos + 1);
		$array[$key] = $value;
	}
	unlink($path);
	return $array;
}

function generatePNG($path, $filename, $height, $width){
	$originalImage = $path.$filename;
	$pos = strrpos($filename, '.');
	if(!$pos) {
		$pos = strlen($filename);
	}
	$newImage = $path.'png/'.substr($filename, 0, $pos).'.png';
	$dimensions = $height.'x'.$width;
	$string = system('convert '.$originalImage.' -resize '.$dimensions.' -size '.$dimensions.' xc:black +swap -gravity center -composite '.$newImage);
}

function openConnection() {
	$connection = mysql_connect('localhost', 'root', 'root');
	if (!$connection) {
   	die('Não foi possível conectar: ' . mysql_error());
	}
	
	mysql_select_db('lungRetriever', $connection);
	if (!$connection) {
   	die ('Não existe banco lungRetriever : ' . mysql_error());
   }
   return $connection;
}

function addCTScan($file) {
	$ct = array();
	$connection = openConnection();
   
   $ct['filename'] = createRandomName(8);
	if (file_exists("ct/" . $ct['filename'])) {
   	die($file["tmp_name"] .	 " already exists. ");
   }
   else {
      move_uploaded_file($file["tmp_name"], "ct/" . $ct['filename']);
   }
   generatePNG('ct/', $ct['filename'], 256, 256);
   
   $result = mysql_query('INSERT INTO CT_slice(filename) VALUES ("'.$ct['filename'].'")', $connection);
	if (!$result) {
		die("Falha ao adicionar no banco.(1)");
	}
	
	$ct['id'] = mysql_insert_id($connection);
	
	system("../Segmentation ct/".$ct['filename']);
	
	if (file_exists("ct/".$ct['filename']."_left.dcm")) {
		rename("ct/".$ct['filename']."_left.dcm", "lungs/".$ct['filename']."_left.dcm");
		generatePNG('lungs/', $ct['filename']."_left.dcm", 120, 120);
		$values = getCharacteristics("ct/".$ct['filename']."_left.txt");
		$result = mysql_query('INSERT INTO lungs(energy,entropy,inverseDifferenceMoment,contrast,correlation)
				VALUES ('.$values['energy'].','.$values['entropy'].','.$values['inverseDifferenceMoment'].','
				.$values['contrast'].','.$values['correlation'].')', $connection);
		if (!$result) {
			die("Falha ao adicionar no banco.(2)");
		}
		
		$id_left = mysql_insert_id($connection);
		$result = mysql_query('UPDATE CT_slice SET leftLung='.$id_left.' WHERE id='.$ct['id'], $connection);
		if (!$result) {
			die("Falha ao adicionar no banco.(3)");
		}
	}
	if (file_exists("ct/".$ct['filename']."_right.dcm")) {
		rename("ct/".$ct['filename']."_right.dcm", "lungs/".$ct['filename']."_right.dcm");
		generatePNG('lungs/', $ct['filename']."_right.dcm", 120, 120);
		$values = getCharacteristics("ct/".$ct['filename']."_right.txt");
		$result = mysql_query('INSERT INTO lungs(energy,entropy,inverseDifferenceMoment,contrast,correlation)
				VALUES ('.$values['energy'].','.$values['entropy'].','.$values['inverseDifferenceMoment'].','
				.$values['contrast'].','.$values['correlation'].')', $connection);
		if (!$result) {
			die("Falha ao adicionar no banco.(4)");
		}
		
		$id_right = mysql_insert_id($connection);
		$result = mysql_query('UPDATE CT_slice SET rightLung='.$id_right.' WHERE id='.$ct['id'], $connection);
		if (!$result) {
			die("Falha ao adicionar no banco.(5)");
		}
	}
	mysql_close($connection);
	return $ct;
}

function getCTScan($id) {
	$ct = array();
	$connection = openConnection();
	
	$result = mysql_query('SELECT * FROM CT_slice WHERE id='.$id, $connection);
	$row = mysql_fetch_array($result);
	if (!$result) {
		die("Falha ao buscar no banco.");
	}
	$ct['filename'] = $row['filename'];	
	$ct['id'] = $id;	
	
	mysql_close($connection);
	return $ct;
}

function similarity($features1, $features2){
	$input = '';
	$input .= $features1['energy'];
	$input .= ' ';
	$input .= $features1['entropy'];
	$input .= ' ';
	$input .= $features1['inverseDifferenceMoment'];
	$input .= ' ';
	$input .= $features1['contrast'];
	$input .= ' ';
	$input .= $features1['correlation'];
	$input .= ' ';
	$input .= $features2['energy'];
	$input .= ' ';
	$input .= $features2['entropy'];
	$input .= ' ';
	$input .= $features2['inverseDifferenceMoment'];
	$input .= ' ';
	$input .= $features2['contrast'];
	$input .= ' ';
	$input .= $features2['correlation'];
	$output = exec('../SimilarityCalculator ../trained.net '.$input);
	return $output;
}

function getSimilarImages($id, $quantity, $left) {
	$similar = array();
	$connection = openConnection();
	
	$result = mysql_query('SELECT l.* FROM lungs l INNER JOIN CT_slice s ON l.id=s.'.($left?'leftLung':'rightLung').' WHERE s.id='.$id, $connection);
	$mainCharacteristics = mysql_fetch_array($result);
	
	$similarities = array();
	$result = mysql_query('SELECT * FROM lungs', $connection);
	while($row = mysql_fetch_array($result)) {
		$value = similarity($mainCharacteristics, $row);
		$similarities[] = array('value' => $value, 'id' => $row['id']); 
	}
	rsort($similarities);
	
	for($i = 0; $i < $quantity && $i < count($similarities); $i++) {
		$result = mysql_query('SELECT filename,leftLung FROM CT_slice WHERE 
					leftLung='.$similarities[$i]['id'].' OR rightLung='.$similarities[$i]['id'], $connection);
		$row = mysql_fetch_array($result);
		if($similarities[$i]['id'] == $row['leftLung']) {
			$similar[$i]['position'] = 'left';
		}
		else {
			$similar[$i]['position'] = 'right';
		}
		$similar[$i]['name'] = $row['filename'];
		$similar[$i]['value'] = $similarities[$i]['value'];
		$similar[$i]['id'] = $similarities[$i]['id'];
	}	
	
	mysql_close($connection);
	return $similar;
}

function getLungId($id, $lado) {
	$connection = openConnection();
	$result = mysql_query('SELECT leftLung,rightLung FROM CT_slice WHERE id='.$id, $connection);
	$row = mysql_fetch_array($result);
	if ($lado == 'left') {
		$lungId = $row['leftLung'];
	}
	else {
		$lungId = $row['rightLung'];
	}
	
	mysql_close($connection);
	return $lungId;
}

function adjustNeuralNetwork($id1, $id2, $adjustFactor) {
	$input = array();
	if ($adjustFactor > 0)
		$adjustFactor = 1;
	else if ($adjustFactor < 0)
		$adjustFactor = 0;
	else 
		$adjustFactor = 0.5;
	
	$connection = openConnection();
	$result = mysql_query('SELECT energy,entropy,inverseDifferenceMoment,contrast,correlation FROM lungs WHERE id='.$id1, $connection);
	$row = mysql_fetch_array($result);
	$input[] = $row['energy'];
	$input[] = $row['entropy'];
	$input[] = $row['inverseDifferenceMoment'];
	$input[] = $row['contrast'];
	$input[] = $row['correlation'];
	$result = mysql_query('SELECT energy,entropy,inverseDifferenceMoment,contrast,correlation FROM lungs WHERE id='.$id2, $connection);
	$row = mysql_fetch_array($result);
	$input[] = $row['energy'];
	$input[] = $row['entropy'];
	$input[] = $row['inverseDifferenceMoment'];
	$input[] = $row['contrast'];
	$input[] = $row['correlation'];
	$input = implode(' ',$input);
	exec('../FineTuner ../trained.net '.$input.' '.$adjustFactor);
	mysql_close($connection);
}

?>
