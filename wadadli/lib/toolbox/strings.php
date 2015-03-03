<?php
	function capitalise($string){
		return strtoupper(substr($string, 0, 1)) . substr($string, 1);
	}
	
	function decapitalise($string){
		return strtolower(substr($string, 0, 1)) . substr($string, 1);	
	}
	
	function isCapitalised($string){
		return $string == capitalise($string);
	}
	
	function camelise($string, $capitalise=false){
		if(!is_string($string)){
			warning(UNEXPECTED_ARGUMENT_WARNING, "camelise() can only convert strings, ".gettype($string)." given.");
			return;
		}
	
		$camelisedString = "";
		foreach(preg_split("/\s+/",$string) as $stringPart){
			$camelisedString .= capitalise($stringPart);
		}
		
		if(!$capitalise){
			$camelisedString = decapitalise($camelisedString);
		}
		
		return $camelisedString;
	}
	
	function decamelise($string, $capitalise=false){
		if(!is_string($string)){
			warning(UNEXPECTED_ARGUMENT_WARNING, "decamelise() can only convert strings, ".gettype($string)." given.");
			return;
		}
	
		$decamelisedString = preg_replace("/([A-Z])/"," $1",$string);
		
		if($capitalise){
			$decamelisedString = capitalise($decamelisedString);
		}else{
			$decamelisedString = strtolower($decamelisedString);
		}
		
		return $decamelisedString;
	}
?>