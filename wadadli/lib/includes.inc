<?php
	require_once(dirname(__FILE__)."/toolbox/files.php");	
	
	//Include Third-Parth Libraries
	require_once(dirname(__FILE__)."/../third_party/addendum/annotations.php");
	
	//Include Wadadli Libraries
	require_once(dirname(__FILE__)."/toolbox/arrays.php");
	require_once(dirname(__FILE__)."/toolbox/files.php");
	require_once(dirname(__FILE__)."/toolbox/log.php");
	require_once(dirname(__FILE__)."/toolbox/messages.php");
	require_once(dirname(__FILE__)."/toolbox/methods.php");
	require_once(dirname(__FILE__)."/toolbox/strings.php");
	require_once(dirname(__FILE__)."/toolbox/test.php");
	require_once(dirname(__FILE__)."/toolbox/Timer.inc");
	require_once(dirname(__FILE__)."/toolbox/wadadli_annotations.php");
	
	require_once(dirname(__FILE__)."/model/reflection/WadadliReflectionClass.inc");
	require_once(dirname(__FILE__)."/model/reflection/WadadliReflectionProperty.inc");
	require_once(dirname(__FILE__)."/model/reflection/WadadliReflectionAnnotation.inc");
	require_once(dirname(__FILE__)."/model/reflection/Mirror.inc");
	
	require_once(dirname(__FILE__)."/worker/Worker.inc");
	require_once(dirname(__FILE__)."/worker/ActionManager.inc");
	require_once(dirname(__FILE__)."/worker/Calligrapher.inc");
	require_once(dirname(__FILE__)."/worker/DatabaseAdministrator.inc");
	require_once(dirname(__FILE__)."/worker/Dispatcher.inc");
	require_once(dirname(__FILE__)."/worker/Janitor.inc");
	require_once(dirname(__FILE__)."/worker/Mason.inc");
	require_once(dirname(__FILE__)."/worker/Postman.inc");
	require_once(dirname(__FILE__)."/worker/QuarterMaster.inc");
	require_once(dirname(__FILE__)."/worker/Registrar.inc");
	require_once(dirname(__FILE__)."/worker/SessionManager.inc");
	require_once(dirname(__FILE__)."/worker/SettingsManager.inc");
	
	require_once(dirname(__FILE__)."/model/WadadliClass.inc");
	require_once(dirname(__FILE__)."/model/PersistentClass.inc");
	
	require_once(dirname(__FILE__)."/controller/Controller.inc");
	require_once(dirname(__FILE__)."/controller/ObjectController.inc");
	require_once(dirname(__FILE__)."/controller/FatalError.inc");
	require_once(dirname(__FILE__)."/controller/Login.inc");
	require_once(dirname(__FILE__)."/controller/Logout.inc");
	
	require_once(dirname(__FILE__)."/model/database/DatabaseConnection.inc");
	require_once(dirname(__FILE__)."/model/database/MySQLConnection.inc");
	require_once(dirname(__FILE__)."/model/database/SQLString.inc");
	
	require_once(dirname(__FILE__)."/model/gateway/Gateway.inc");
	require_once(dirname(__FILE__)."/model/gateway/XMLGateway.inc");
	require_once(dirname(__FILE__)."/model/gateway/SQLGateway.inc");
	require_once(dirname(__FILE__)."/model/gateway/ActionsGateway.inc");
	require_once(dirname(__FILE__)."/model/gateway/SettingsGateway.inc");
	require_once(dirname(__FILE__)."/model/gateway/Criterion.inc");
	require_once(dirname(__FILE__)."/model/gateway/Searcher.inc");
	require_once(dirname(__FILE__)."/model/gateway/Sorter.inc");
	
	require_once(dirname(__FILE__)."/model/http/WadapiRequest.inc");
	require_once(dirname(__FILE__)."/model/http/WadapiResponse.inc");
	
	require_once(dirname(__FILE__)."/model/system/Action.inc");
	require_once(dirname(__FILE__)."/model/system/Settings.inc");
	require_once(dirname(__FILE__)."/model/system/URLPattern.inc");
	require_once(dirname(__FILE__)."/model/system/WadadliDate.inc");
	require_once(dirname(__FILE__)."/model/system/WadadliMessage.inc");
	
	require_once(dirname(__FILE__)."/model/session/Session.inc");
	require_once(dirname(__FILE__)."/model/session/User.inc");
	
	require_once(dirname(__FILE__)."/view/components/WadadliComponent.inc");
	require_once(dirname(__FILE__)."/view/components/WadadliContainer.inc");
	
	require_once(dirname(__FILE__)."/view/components/system/WadadliStringComponent.inc");
	require_once(dirname(__FILE__)."/view/components/system/WadadliImageComponent.inc");
	require_once(dirname(__FILE__)."/view/components/system/WadadliScriptComponent.inc");
	require_once(dirname(__FILE__)."/view/components/system/WadadliSystemMessageBox.inc");
	
	require_once(dirname(__FILE__)."/view/components/layout/WadadliStretcher.inc");
	require_once(dirname(__FILE__)."/view/components/layout/WadadliHeading.inc");
	require_once(dirname(__FILE__)."/view/components/layout/WadadliListComponent.inc");
	require_once(dirname(__FILE__)."/view/components/layout/WadadliListing.inc");
	require_once(dirname(__FILE__)."/view/components/layout/WadadliTable.inc");
	
	require_once(dirname(__FILE__)."/view/components/input/WadadliInputComponent.inc");
	require_once(dirname(__FILE__)."/view/components/input/WadadliButton.inc");
	require_once(dirname(__FILE__)."/view/components/input/WadadliCheckbox.inc");
	require_once(dirname(__FILE__)."/view/components/input/WadadliDropDown.inc");
	require_once(dirname(__FILE__)."/view/components/input/WadadliFileField.inc");
	require_once(dirname(__FILE__)."/view/components/input/WadadliForm.inc");
	require_once(dirname(__FILE__)."/view/components/input/WadadliImageField.inc");
	require_once(dirname(__FILE__)."/view/components/input/WadadliInputContainer.inc");
	require_once(dirname(__FILE__)."/view/components/input/WadadliRadioPanel.inc");
	require_once(dirname(__FILE__)."/view/components/input/WadadliTextBox.inc");
	require_once(dirname(__FILE__)."/view/components/input/WadadliTextField.inc");
	require_once(dirname(__FILE__)."/view/components/input/WadadliColourPicker.inc");
	require_once(dirname(__FILE__)."/view/components/input/WadadliDatePicker.inc");
	require_once(dirname(__FILE__)."/view/components/input/WadadliHiddenField.inc");
	require_once(dirname(__FILE__)."/view/components/input/WadadliPasswordField.inc");
	require_once(dirname(__FILE__)."/view/components/input/WadadliPasswordConfirmationField.inc");
	require_once(dirname(__FILE__)."/view/components/input/WadadliRatingStars.inc");
	
	require_once(dirname(__FILE__)."/view/components/navigation/WadadliMenu.inc");
	require_once(dirname(__FILE__)."/view/components/navigation/WadadliAnchor.inc");
	require_once(dirname(__FILE__)."/view/components/navigation/WadadliActionLink.inc");
	require_once(dirname(__FILE__)."/view/components/navigation/WadadliEmailAnchor.inc");
	require_once(dirname(__FILE__)."/view/components/navigation/WadadliPageSelect.inc");
	require_once(dirname(__FILE__)."/view/components/navigation/WadadliObjectPageSelect.inc");
	
	require_once(dirname(__FILE__)."/view/View.inc");
	require_once(dirname(__FILE__)."/view/ColumnView.inc");
	require_once(dirname(__FILE__)."/view/ErrorView.inc");
	require_once(dirname(__FILE__)."/view/LoginView.inc");
	
	require_once(dirname(__FILE__)."/view/WadadliPageTemplate.inc");
		
	//Import activated modules
	foreach(explode(",",SettingsManager::getSetting("modules","enabled")) as $module){
		require_all(SettingsManager::getSetting("install","projectPath")."/wadadli/modules/$module");
	}
	
	//Include Wadadli Project Files
	require_all(SettingsManager::getSetting("install","projectPath")."/controller");
	require_all(SettingsManager::getSetting("install","projectPath")."/model");
	require_all(SettingsManager::getSetting("install","projectPath")."/view");
?>