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
			source: itemModel? ("image://imageprovider/atom-"+itemModel.itemRow) : "";
			//asynchronous: true; //fast but looks ugly

			Connections {
				target: mainWidget
				onUpdateHeatMap: { if(itemModel){ heatmap.source = ""; heatmap.source = ("image://imageprovider/atom-"+itemModel.itemRow);}}
			}
		}
		
		Rectangle {
			color: "transparent"
			anchors.fill: parent;
			border.color: (itemModel && atomsData.selectedAtom == itemModel.itemRow)? "#ff9c00" : colorLib.highlight; 
			border.width: (itemModel && (atomsData.hoveredAtom == itemModel.itemRow || atomsData.selectedAtom == itemModel.itemRow))?1:0;
			
			MouseArea {
				hoverEnabled: true;
				anchors.fill: parent;
				acceptedButtons: Qt.LeftButton | Qt.RightButton; //Qt.LeftButton ; //Qt.AllButtons;| Qt.RightButton
				onClicked:{
					if(atomsData.selectedAtom == itemModel.itemRow)
						atomsData.selectedAtom = -1;
					else{
						atomsData.selectedAtom = itemModel.itemRow;
					}
					mouse.accepted = true;
				}
				onEntered:{
					atomsData.hoveredAtom  = itemModel.itemRow;
				}
				onExited:{
					atomsData.hoveredAtom = -1;
					infobox.x = -100;
					infobox.y = -100;
				}
				onPositionChanged: {
					root.hideText = mouse.x < 200;
					
					var coords = this.mapToItem(root, mouse.x, mouse.y);
					var frame = Math.floor(timeline.start + (coords.x/ruler.width) * (timeline.end-timeline.start));
					var atomlayer = atomsData.getAtomLayer(itemModel.itemRow, frame);
					if(atomlayer != -1){
						infobox.layerVal = atomlayer;
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