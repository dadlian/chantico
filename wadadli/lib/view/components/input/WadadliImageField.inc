<?php
	class WadadliImageField extends WadadliInputComponent{
		/** @Float(required=true,default=250) */
		protected $width;
		
		/** @Float(required=true,default=250) */
		protected $height;
	
		protected function getEntryComponent(){
			$componentTitle = decapitalise(str_replace(" ","",$this->getTitle()));
			
			$toggleImageMenu = new WadadliImageComponent("","wadadli/img/edit.png","Edit");
			$toggleImageMenu->addClass('toggleImageMenu');
			$toggleImageMenu->onClick('toggleImageMenu');
			
			$imageUploadLink = new WadadliContainer();
			$imageUploadLink->setTag('span');
			$imageUploadLink->addContent(new WadadliStringComponent("Upload"));
			$imageUploadLink->addClass('menuItem');
			
			$imageInput = new WadadliComponent($this->getTitle());
			$imageInput->setTag('input');
			$imageInput->addAttribute('type','file');
			$imageInput->addAttribute('name',$componentTitle);
			
			$imageUpload = new WadadliContainer();
			$imageUpload->addClass('imageUpload');
			$imageUpload->onChange('uploadImage');
			
			$imageUpload->addContent($imageUploadLink);
			$imageUpload->addContent($imageInput);
			
			$imageDeleteLink = new WadadliContainer("{$componentTitle}DeleteLink");
			$imageDeleteLink->setTag('span');
			$imageDeleteLink->addContent(new WadadliStringComponent("Delete"));
			$imageDeleteLink->addClass('menuItem');
			$imageDeleteLink->onClick('deleteImage');
			
			$imageDelete = new WadadliHiddenField(preg_replace("/([A-Z])/"," $1","{$componentTitle}Delete"));
			
			$imageMenu = new WadadliContainer("{$componentTitle}Menu");
			$imageMenu->addClass("imageUploadMenu");
			$imageMenu->addContent($imageUpload);
			$imageMenu->addContent($imageDeleteLink);
			$imageMenu->addContent($imageDelete);
			$imageMenu->addClass('hidden');
			
			$imageMenuArea = new WadadliContainer();
			$imageMenuArea->addClass('imageMenuArea');
			$imageMenuArea->addContent($toggleImageMenu);
			$imageMenuArea->addContent($imageMenu);
			
			$uploadedImage = SettingsManager::getSetting("install","projectPath")."/".$this->getDefaultValue();
			$imageDimensions = getimagesize($uploadedImage);
			if($this->getWidth() <= $imageDimensions[0] || $imageDimensions[0] <= $imageDimensions[1]){
				$scaledWidth = $this->getWidth();
				$scaledHeight = min($scaledWidth * ($imageDimensions[1]/$imageDimensions[0]),$this->getHeight());
			}else{
				$scaledHeight = $this->getHeight();
				$scaledWidth = min($scaledHeight * ($imageDimensions[0]/$imageDimensions[1]), $this->getWidth());
			}
			
			$verticalPadding = max(0,($this->getHeight()-$scaledHeight)/2);
			
			$imagePreview = new WadadliContainer();
			$imagePreview ->addClass('imagePreview');
			$imagePreview ->addAttribute("style","width:{$this->getWidth()}px; height:{$this->getHeight()}px;padding:{$verticalPadding}px 0px");
			
			$uploadedImageComponent = new WadadliImageComponent("",$this->getDefaultValue(),"{$this->getTitle()} Preview");
			$uploadedImageComponent->addAttribute("style","width:{$scaledWidth}px;height:{$scaledHeight}px");
			$imagePreview->addContent($uploadedImageComponent);
			
			$newImage = new WadadliTextField();
			$newImage->addEntryAttribute('readonly','readonly');
			$newImage->addEntryAttribute('style',"width:{$this->getWidth()}px");
			
			$entryComponent = new WadadliContainer("{$this->getComponentId()}ImageField");
			$entryComponent->addContent($imageMenuArea);
			$entryComponent->addContent($imagePreview);
			$entryComponent->addContent($newImage);
			$entryComponent->addClass("imageSelector");
			
			return $entryComponent;
		}
	}
?>