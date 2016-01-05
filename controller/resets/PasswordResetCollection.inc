<?php
	class PasswordResetCollection extends PasswordResetController{
		protected function post(){
			$sqlGateway = new SQLGateway();
			$searcher = new Searcher();
			
			//Close any open reset requests
			$searcher->addCriterion("status",Criterion::EQUAL,"requested");
			$openResets = array();
			foreach($sqlGateway->find("PasswordReset",$searcher) as $openReset){
				$openReset->setStatus("cancelled");
				$openResets[] = $openReset;
			}
			$sqlGateway->save($openResets);
			
			//Create New Password Reset Request
			$reset = new PasswordReset();
			$reset->setUser($this->viewFromArguments("user"));
			$reset->setCode(rand(100000,999999));
			$reset->setStatus("requested");
			$sqlGateway->save($reset);


			//Send Confirmation Code via Email
			$template = "<html>
					<head>
						<title>Your Password Reset Instructions</title>
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
										You have requested that your TickeTing password be reset. To
										verify that you have initiated this request and to confirm
										your new password, please enter the confirmation code below
										when prompted in the app.
									</p>
									<h3>Your confirmation code:</h3>
									<h1 style='color:#2072b8;'>{$reset->getCode()}</h1>
									<p>
										Once you have done so, your new password will take effect immediately.
									</p>
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
			
			$url = "https://api:key-ade3853dbd19ecf3efe099367871590e@api.mailgun.net/v3/myticketting.com/messages";
			$payload = array(
				"to"=> "{$this->getFromContent('recipient')} <{$this->getFromContent('email')}>",
				"from"=> "The TickeTing Team <noreply@myticketting.com>",
				"subject"=>"Password Reset Confirmation Code",
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
			
			$payload = $this->assemblePayload($reset);
			ResponseHandler::created($payload,$reset->getURI());
		}
		
		protected function isConsistent($modifiedDate,$eTag){
			return true;
		}
	}
?>