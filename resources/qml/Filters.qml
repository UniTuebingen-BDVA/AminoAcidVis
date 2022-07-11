import QtQuick 2.4

Rectangle { 
	//width: parent.width
	id: root
	color: colorLib.window
	Flow {
		anchors.fill: parent
		
		anchors.margins: 4
		spacing: 2
		
		Image {
			source: "img/start_cicle.png"
		}
					
		Repeater {
				model: filters
				
				Row{
					spacing: 2
					Image {
						source: "img/next_arrow.png"
					}
					
					Rectangle {
						color: "#9c3122"
						radius: 3
							
						MouseArea {
							anchors.fill: parent;
							acceptedButtons: Qt.RightButton;
							onClicked:{
								filterMenu.popup();
							}
						}
						
						Row{
							y:1
							//x:3
							height: 23
							spacing: 2
							
							Column{
								y: -1
								spacing: 0
								Image {
									source: (model.renderViewEnabled)? "img/filter_switch_R_off.png": "img/filter_switch_R_on.png";
									MouseArea {
										anchors.fill: parent;
										acceptedButtons: Qt.LeftButton ;
										onClicked:{
											parent.source = (model.renderViewEnabled)? "img/filter_switch_R_on.png": "img/filter_switch_R_off.png";
											filtersProxy.disableRenderView(model.index, !model.renderViewEnabled);
										}
									}
								}
								Image {
									source: (model.timelineViewEnabled)? "img/filter_switch_T_off.png": "img/filter_switch_T_on.png";
									MouseArea {
										anchors.fill: parent;
										acceptedButtons: Qt.LeftButton ;
										onClicked:{
											parent.source = (model.timelineViewEnabled)? "img/filter_switch_T_on.png": "img/filter_switch_T_off.png";
											filtersProxy.disableTimeline(model.index, !model.timelineViewEnabled);
										}
									}
								}
							}

							
							Text { 
								verticalAlignment: Text.AlignHCenter
								color: colorLib.text; text: display; 
								font.pixelSize: (display.length > 5)? 12 : 17 //17 
								y: (display.length > 5)? 4 : 0
							}
							
							Image {
								id: deleteButton
								y:1
								source: "img/close.png"
								
								MouseArea {
									anchors.fill: parent;
									hoverEnabled: true;
									acceptedButtons: Qt.LeftButton ; //Qt.AllButtons;| Qt.RightButton
									onClicked:{
										filtersProxy.removeFilter(model.index);
									}
									onEntered:{
										parent.source = "img/close_hover.png";
									}
									onExited:{
										parent.source = "img/close.png";
									}
								}
							}
						}
						
						width: childrenRect.width
						height: childrenRect.height
					}
				}
		}
	}
}