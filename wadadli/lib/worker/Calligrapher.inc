<?php
	class Calligrapher extends Worker{
		public static $visualNewline = "&S3DF38NS84CDS0S";
		
		public static function formatMarkup($string){
			$indent = 0;
			$previousClosingTag = "";
			$closingTag = false;
			$insertedCharacters = 0;
			
			preg_match_all("/<.*?>/",$string,$matches,PREG_OFFSET_CAPTURE);
			foreach($matches[0] as $match){
				$tag = $match[0];
				$offset = $match[1] + $insertedCharacters;
				
				$closingTag = preg_match("/<\//", $tag);
				
				//Detemine if there is a closing tag and indentation decrease
				if($closingTag && $indent){
					$indent--;
				}
				
				//Determine if there is an indent and newline
				if(!$closingTag || $previousClosingTag){
					//$string = substr($string, 0, $offset)."\n".str_repeat("\t",$indent).substr($string, $offset);
					$insertedCharacters += ($indent+1);
				}
				
				//Determine if there is am opening tag and indentation increase
				if(!$closingTag){
					$indent++;
				}
				
				$previousTag = $tag;
				$previousClosingTag = $closingTag;
			}
			
			$formattedString = preg_replace("/\n\s*\n/","\n",trim($string));
			return $formattedString;
		}
	}
?>