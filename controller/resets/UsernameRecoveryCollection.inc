<?php
	class UsernameRecoveryCollection extends UsernameRecoveryController{
		protected function post(){
			$sqlGateway = new SQLGateway();
			$searcher = new Searcher();
			$user = $this->getResourceObject("ManagedAccess","id",$this->viewFromArguments("user"));

			//Create New Username Recovery Request
			$recovery = new UsernameRecovery();
			$recovery->setUser($this->viewFromArguments("user"));
			$sqlGateway->save($recovery);


			//Send Confirmation Code via Email
			$template = "<html>
					<head>
						<title>Username Recovery</title>
					</head>
					<body>
						<table width='500px' style='font-family:Arial, Helvetica, sans-serif;font-size:13px;'>
							<tr>
								<td>
									<img src='http://nkengedrew.com/client/ticketing/tickeTingLogo.jpg' alt='TickeTing logo' style='border:none;'>
									<h3>Dear ".$this->getFromContent("recipient").",</h3>
								</td>
							</tr>
							<tr>
								<td>
									<p>
										You requested your TickeTing username be sent to your registered email address. Please find it included below.
									</p>
									<h3>Your username:</h3>
									<h1 style='color:#2072b8;'>{$user->getUsername()}</h1>
									<p>
										<span>Sincerely,<br></span>
										<span style='color:#2072b8;'><strong>The TickeTing Team</strong></span>
									</p>
								</td>
							</tr>
							<tr>
								<td height='20px'>
								</td>
							</tr>
							<tr>
								<td bgcolor='#2072b8' style='color:#ffffff;padding:10px;font-size:12px;text-align:center;' >
									&copy; ".date("Y")." mCAT Innovations, Antigua, W.I.
								</td>
							</tr>
						</table>
					</body>
				<html>";

			$apiKey = SettingsManager::getSetting("mailgun","ticketingkey");
			$url = "https://api:key-{$apiKey}@api.mailgun.net/v3/ticketingevents.co/messages";
			$payload = array(
				"to"=> "{$this->getFromContent('recipient')} <{$this->getFromContent('email')}>",
				"from"=> "The TickeTing Team <noreply@ticketingevents.co>",
				"subject"=>"Your TickeTing Username",
				"html"=> $template
			);

			$options = array(
				'http' => array(
					'header'  => "Content-type: application/x-www-form-urlencoded\r\n",
					'method'  => 'POST',
					'content' => http_build_query($payload),
				)
			);
			$context  = stream_context_create($options);
			$result = file_get_contents($url, false, $context);

			$payload = $this->assemblePayload($recovery);
			ResponseHandler::created($payload,$recovery->getURI());
		}

		protected function isConsistent($modifiedDate,$eTag){
			return true;
		}
	}
?>
