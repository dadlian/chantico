<?php
	class WadapiResponse extends WadadliClass{
		/** @Integer */
		protected $statusCode;
		
		/** @String */
		protected $reason;
		
		/** @Collection(type=@String) */
		protected $headers;
		
		/** @String */
		protected $contentType;
		
		/** @String */
		protected $contentCharset;
		
		/** @String */
		protected $contentLanguage;
		
		/** @String */
		protected $body;
		
		protected function setContentURI($contentLocation,$includeLocation=true){
			$this->insertToHeaders("Content-Location",$contentLocation);
			if($includeLocation){
				$this->insertToHeaders("Location",$contentLocation);
			}
		}
		
		protected function setState($modified,$eTag){
			$httpDate = gmdate("D, d M Y H:i:s",$modified)." GMT";
			$this->insertToHeaders("Last-Modified",$httpDate);
			$this->insertToHeaders("ETag",$eTag);
		
		}
		
		protected function setExpiryDate($date){
			$httpDate = gmdate("D, d M Y H:i:s",$date)." GMT";
			$this->insertToHeaders("Expires",$httpDate);
		}
		
		protected function send(){			
			//Build Response
			$response = "";
			
			//Configure Request Line
			http_response_code($this->getStatusCode());
			
			//Build Headers
			header("Date: ".gmdate("D, d M Y H:i:s")." GMT");
			foreach($this->getHeaders() as $header => $value){
				header("$header: $value");
			}
			
			//Build Request Body
			if($this->getBody()){
				header("Content-Type: {$this->getContentType()};charset={$this->getContentCharset()}");
				header("Content-Language: {$this->getContentLanguage()}");
				header("Content-Length: ".strlen($this->getBody()));
			}
			
			echo $this->getBody();
		}
	}
?>