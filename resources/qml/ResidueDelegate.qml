import QtQuick 2.4

Component{
	Rectangle {  
		id:componentRoot
		color: colorLib.window //colorLib.window
		width: ruler.width;
		height: 25 
		
		Image{
			id: heatmap
			width: ruler.width;
			height: parent.height;
			cache: false;
			sourceSize.width: ruler.width;
			sourceSize.height: parent.height;
			source: itemModel? ("image://imageprovider/residue-"+itemModel.itemRow) : "";
			asynchronous: true; //fast but looks ugly

			Connections {
				target: mainWidget
				onUpdateHeatMap: { 
					if(itemModel){ heatmap.source = ""; heatmap.source = ("image://imageprovider/residue-"+itemModel.itemRow);}}
				}
		}
		
		Rectangle {
			color: "transparent"
			anchors.fill: parent;
			border.color: (itemModel && atomsData.selectedGroup == itemModel.itemRow)? "#ff9c00" : colorLib.highlight; 
			border.width: (itemModel && (atomsData.hoveredGroup == itemModel.itemRow+1 || atomsData.selectedGroup == itemModel.itemRow))? 1: 0;
			
			
			MouseArea {
				hoverEnabled: true;
				anchors.fill: parent;
				acceptedButtons: Qt.LeftButton | Qt.RightButton; //Qt.AllButtons;| Qt.RightButton
				onClicked:{
					if(mouse.button & Qt.LeftButton){
						var qIndex = atomsFiltered.mapFromSource(itemModel.itemIndex)
						if(dataView.isExpanded(qIndex))
							dataView.collapse(qIndex); 
						else
							dataView.expand(qIndex); 
					}else{
						if(atomsData.selectedGroup == itemModel.itemRow){
							atomsData.selectedGroup = -1;
						}else
							atomsData.selectedGroup = itemModel.itemRow;
					}
				}
				onEntered:{
					atomsData.hoveredGroup = itemModel.itemRow+1;
				}
				onExited:{
					atomsData.hoveredGroup = -1;
					infobox.x = -100;
					infobox.y = -100;
				}
				onPositionChanged: {
					root.hideText = mouse.x < 200;
					
					var coords = this.mapToItem(root, mouse.x, mouse.y);
					var frame = Math.floor(timeline.start + (coords.x/ruler.width) * (timeline.end-timeline.start));
					var avarage = atomsData.getGroupLayer(itemModel.itemRow, frame);
					if(avarage != -1){
						infobox.layerVal = avarage;
						infobox.x = (coords.x+10 > root.width-infobox.width)? coords.x-infobox.width :coords.x+5;
						infobox.y = coords.y+10;
					}else{
						infobox.x = -100;
						infobox.y = -100;
					}
				}
			}
			
		}
		
	}
}