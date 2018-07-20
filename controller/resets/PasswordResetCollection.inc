<?php
	class PasswordResetCollection extends PasswordResetController{
		protected function post(){
			$sqlGateway = new SQLGateway();
			$searcher = new Searcher();

			//Close any open reset requests
			$searcher->addCriterion("status",Criterion::EQUAL,"requested");
			$searcher->addCriterion("user",Criterion::EQUAL,$this->viewFromArguments("user"));
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

			$searcher->clearCriteria();
			$searcher->addCriterion("id",Criterion::EQUAL,$reset->getUser());
			$managedAccess = $sqlGateway->findUnique("ManagedAccess",$searcher);

			//Send Confirmation Code via Email
			if($managedAccess->getApi() == 14358019264956){
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

				$apiKey = SettingsManager::getSetting("mailgun","ticketingkey");
				$url = "https://api:key-{$apiKey}@api.mailgun.net/v3/ticketingevents.co/messages";
				$payload = array(
					"to"=> "{$this->getFromContent('recipient')} <{$this->getFromContent('email')}>",
					"from"=> "The TickeTing Team <noreply@ticketingevents.co>",
					"subject"=>"Password Reset Confirmation Code",
					"html"=> $template
				);
			}else if(in_array($managedAccess->getApi(),[14915827114381,14915100262884])){
				$template = "<html>
					<head>
						<title>ACYM Password Reset</title>
					</head>
					<body>
						<table>
							<tr>
								<td>
									<img style='float:left; width:25%;' src='https://registration.antiguacharteryachtmeeting.com/assets/email_logo.jpg' alt='ACYM logo' style='border:none;'>
								</td>
							</tr>
							<tr>
								<td>
									<p style='font-size:14px; font-family:Verdana, sans-serif;'>
										You have requested that your ACYM password be reset. To
										verify that you have initiated this request and to confirm
										your new password, please enter the confirmation code below
										when prompted on the website.
									</p>
								</td>
							</tr>
							<tr>
								<td>
									<h3 style='font-family:Verdana, sans-serif;'>Your confirmation code:</h3>
									<h1 style='font-family:Verdana, sans-serif; color:#1589BF'>{$reset->getCode()}</h1>
								</td>
							</tr>
							<tr>
								<td>
									<p style='font-size:14px; font-family:Verdana, sans-serif;'>
										Once you have done so, your new password will take effect immediately.
									</p>
								</td>
							</tr>
							<tr>
								<td height='20px'>
								</td>
							</tr>
							<tr>
								<td>
									<div>Regards,</div>
									<div>Antigua Charter Yacht Meeting</div>
									<div>Tel.: +1 (268) 460-1059</div>
									<div><a href='http://antiguayachtshow.com'>http://antiguayachtshow.com</a></div>
								</td>
							</tr>
							<tr>
								<td><hr /></td>
							</tr>
							<tr>
								<td bgcolor='#1589BF' style='color:#FFFFFF;padding:10px;font-size:12px;text-align:center;'>
										&copy; ".date('Y')." Antigua Charter Yacht Meeting, Antigua, W.I.
								</td>
							</tr>
						</table>
					</body>
				<html>";

				$url = "https://api:key-4d5d0be711735d4830f0ea72a38b0053@api.mailgun.net/v3/antiguacharteryachtmeeting.com/messages";
				$payload = array(
					"to"=> "{$this->getFromContent('recipient')} <{$this->getFromContent('email')}>",
					"from"=> "Antigua Charter Yacht Meeting <noreply@antiguacharteryachtmeeting.com>",
					"subject"=>"Your ACYM Account Password Reset",
					"html"=> $template
				);
			}

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
