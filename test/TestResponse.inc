<?php
	class TestResponse{
		protected $statusCode;
		protected $reason;
		protected $headers;
		protected $body;
		
		public function __construct(){
			$this->statusCode = 200;
			$this->reason = "OK";
			$this->headers = array();
			$this->contentType = "";
			$this->contentCharset = "";
			$this->contentLanguage = "";
			$this->body = "";
		}
		
		public function getStatusCode(){
			return $this->statusCode;
		}
		
		public function setStatusCode($newStatusCode){
			$this->statusCode = $newStatusCode;
		}
		
		public function getReason(){
			return $this->reason;
		}
		
		public function setReason($newReason){
			$this->reason = $newReason;
		}
		
		public function getHeaders($includeAccessControl=true){
			$headers = array();
			foreach($this->headers as $header => $value){
				if($includeAccessControl){
					$headers[$header] = $value;
				}else if(!preg_match("/^Access-Control-/",$header)){
					$headers[$header] = $value;
				}
			}
		
			return $headers;
		}
		
		public function viewFromHeaders($header){
			if(array_key_exists($header,$this->headers)){
				return $this->headers[$header];
			}else{
				return "";
			}
		}
		
		public function insertToHeaders($newHeader,$newValue){
			$this->headers[$newHeader] = $newValue;
		}
		
		public function getBody(){
			return $this->body;
		}
		
		public function setBody($newBody){
			$this->body = $newBody;
		}
		
		protected function setContentURI($contentLocation,$includeLocation=true){
			$this->headers["Content-Location"] = $contentLocation;
			if($includeLocation){
				$this->headers["Location"] =  $contentLocation;
			}
		}
		
		protected function setState($modified,$eTag){
			$httpDate = gmdate("D, d M Y H:i:s",$modified)." GMT";
			$this->headers["Last-Modified"] = $httpDate;
			$this->headers["ETag"] = $eTag;
		
		}
		
		protected function setExpiryDate($date){
			$httpDate = gmdate("D, d M Y H:i:s",$date)." GMT";
			$this->headers["Expires"] = $httpDate;
		}
		
		protected function send(){			
			//Build Response
			$response = "";
			
			//Configure Request Line
			http_response_code($this->statusCode);
			
			//Build Headers
			header("Date: ".gmdate("D, d M Y H:i:s")." GMT");
			foreach($this->headers as $header => $value){
				header("$header: $value");
			}
			
			//Build Request Body
			if($this->body){
				header("Content-Type: {$this->contentType};charset={$this->contentCharset}");
				header("Content-Language: {$this->contentLanguage}");
				header("Content-Length: ".strlen($this->body));
			}
			
			echo $this->body;
		}
	}
?>