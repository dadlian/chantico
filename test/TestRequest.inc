<?php
	class TestRequest{
		protected $host;
		protected $endpoint;
		protected $method;
		protected $arguments;
		protected $headers;
		protected $contentType;
		protected $contentLength;
		protected $body;
		protected $transport;
		protected $port;
		private $addedResources;
		private $removedResources;
		
		public function __construct($host){
			$this->host = $host;
			$this->endpoint = "/";
			$this->method = "GET";
			$this->arguments = array();
			$this->headers = array();
			$this->contentType = "text/plain";
			$this->body = "";
			$this->transport = "tcp";
			$this->port = "80";
			$this->addedResources = array();
			$this->removedResources = array();
		}
		
		public function setEndpoint($newEndpoint){
			$this->endpoint = $newEndpoint;
		}
		
		public function setContentType($newContentType){
			$this->contentType = $newContentType;
		}
		
		public function setContentLength($newContentLength){
			$this->contentLength = $newContentLength;
		}
		
		public function setBody($newBody){
			$this->body = $newBody;
		}
		
		public function takeFromHeaders($header){
			if(array_key_exists($header,$this->headers)){
				unset($this->headers[$header]);
			}
		}
		
		public function insertToArguments($argument,$value){
			$this->arguments[$argument] = $value;
		}
		
		public function clearArguments(){
			$this->arguments = array();
		}
		
		public function addPreferedContentType($mimeType,$priority=1.0){
			$this->addPreference("Accept",$mimeType,$priority);
		}
		
		public function addPreferedCharset($mimeType,$priority=1.0){
			$this->addPreference("Accept-Charset",$mimeType,$priority);
		}
		
		public function addPreferedEncoding($mimeType,$priority=1.0){
			$this->addPreference("Accept-Encoding",$mimeType,$priority);
		}
		
		public function addPreferedLanguage($mimeType,$priority=1.0){
			$this->addPreference("Accept-Language",$mimeType,$priority);
		}
		
		public function authorise($key,$secret){
			$this->headers["Authorization"] = "Basic ".base64_encode("$key:$secret");
		}
		
		public function assureCurrency($date,$eTag){
			$httpDate = gmdate("D, d M Y H:i:s",strtotime($date))." GMT";
			$this->headers["If-Modified-Since"] = $httpDate;
			$this->headers["If-None-Match"] = $eTag;
		}
		
		public function assureConsistency($date=null,$eTag=null){
			if(!$date || !$eTag){
				$this->setBody("");
				$response = $this->get();
				$date = $response->viewFromHeaders("Last-Modified");
				$eTag = $response->viewFromHeaders("ETag");
			}
			
			$httpDate = gmdate("D, d M Y H:i:s",strtotime($date))." GMT";
			$this->headers["If-Unmodified-Since"] = $httpDate;
			$this->headers["If-Match"] = $eTag;
		}
		
		public function suggestURI($suggestion){
			$this->headers["Slug"] = $suggestion;
		}
		
		public function get(){
			$this->method = "GET";
			return $this->send();
		}
		
		public function post(){
			$this->method = "POST";
			$response = $this->send();
			
			$body = json_decode($response->getBody(),true);
			if(json_last_error() == JSON_ERROR_NONE){
				if(array_key_exists("self",$body)){
					$this->addedResources[] = preg_replace("/(http\:\/\/)?{$this->host}/","",$body["self"]);
				}
			}
			
			return $response;
		}
		
		public function put(){
			$this->method = "PUT";
			return $this->send();
		}
		
		public function delete(){
			$this->method = "DELETE";
			$response = $this->send();
			
			$body = json_decode($response->getBody(),true);
			if(json_last_error() == JSON_ERROR_NONE){
				if(array_key_exists("self",$body)){
					$this->removedResources[] = preg_replace("/(http\:\/\/)?{$this->host}/","",$body["self"]);
				}
			}
			
			return $response;
		}
		
		public function flushTestResources($key,$secret){
			$deleteRequest = new TestRequest($this->host);
			$deleteRequest->authorise($key,$secret);
			$addedResources = $this->addedResources;
			arsort($addedResources);
			
			foreach(array_diff($addedResources,$this->removedResources) as $resource){
				$deleteRequest->setEndpoint($resource);
				$deleteRequest->assureConsistency();
				$deleteRequest->delete();
			}
			
			$this->addedResources = array();
		}
		
		private function send(){
			//Establish a Connection to the Remote Host
			$destination = "{$this->transport}://{$this->host}:{$this->port}";
			$remoteConnection = stream_socket_client($destination,$errorNumber,$errorString);
			
			if(!$remoteConnection){
				fatal_error("There was a problem establishing a remote connection.","$errorNumber: $errorString");
			}
			
			//Build Request
			$request = "";
			
			//Build the URL query string
			$queryString = "";
			if($this->arguments){
				$queryString = array();
				foreach($this->arguments as $key=>$value){
					$queryString[] = "$key=".urlencode($value);
				}
				$queryString = "?".implode("&",$queryString);
			}
			
			//Build Request Line
			$singleSlashEndpoint = preg_replace("/\/+/","/","/{$this->endpoint}");
			$request .= "$this->method $singleSlashEndpoint$queryString HTTP/1.1\r\n";
			
			//Build Headers
			$request .= "Host: {$this->host}\r\n";
			$request .= "Date: ".gmdate("D, d M Y H:i:s")." GMT\r\n";
			foreach($this->headers as $header => $value){
				$request .= "$header: $value\r\n";
			}
			
			//Build Request Body
			if(ctype_print($this->body)){
				$this->setContentLength(strlen($this->body));
			}
			
			if($this->body){
				$request .= "Content-Type: {$this->contentType}\r\n";
				$request .= "Content-Length: ".$this->contentLength."\r\n";
			}
			
			$request .= "Connection: close\r\n\r\n";
			$request .= $this->body;
			
			echo "\n\n============================================================================================================================\n";
			echo $request;
			echo "\n============================================================================================================================\n\n";
			
			$response = new TestResponse();
			if($remoteConnection){
				//Send Request to server
				fwrite($remoteConnection,$request);
				
				//Parse Response
				$responseString = "";
				while(!feof($remoteConnection)){
					$responseString .= fgets($remoteConnection, 1024);
				}
				
				echo "\n\n============================================================================================================================\n";
				echo $responseString;
				echo "\n============================================================================================================================\n\n";
				
				preg_match("/^(.*)\r\n((?:.*\r\n)*)\r\n((?:.*\n?)*)/",$responseString,$matches);
				if($matches && preg_match("/^HTTP\/1\.[0-1] [0-9]{3} [A-Za-z\s]+$/",$matches[1])){
					$statusParts = preg_split("/\s/",$matches[1]);
					$response->setStatusCode(intval($statusParts[1]));
					$response->setReason(implode(" ",array_slice($statusParts,2,sizeof($statusParts))));
					
					foreach(preg_split("/\r\n/",trim($matches[2])) as $header){
						$headerParts = preg_split("/:\s/",$header);
						$response->insertToHeaders($headerParts[0],trim($headerParts[1]));
					}
					$response->setBody($matches[3]);
				}else{
					warning("An HTTP Response was invalid.","Invalid Response: $responseString");
				}
				
				//Close connection
				fclose($remoteConnection);
			}
			
			return $response;
		}
		
		private function addPreference($header,$preference,$priority){
			$contentTypes = array_key_exists($header,$this->headers)?explode(",",$this->headers[$header]):array();
			
			if(!$priority){
				$priority = 1.0;
			}
			
			$priority = min(max($priority,0.0),1.0);
			$contentTypes[] = sprintf("$preference;q=%.1f",$priority);
			$this->headers[$header] = implode(",",$contentTypes);
		}
	}
?>