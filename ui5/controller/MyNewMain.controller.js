sap.ui.define(['rootui5/eve7/controller/Main.controller',
               'rootui5/eve7/lib/EveManager',
               "sap/ui/core/mvc/XMLView",
               'sap/ui/core/Fragment'
], function(MainController, EveManager, XMLView, Fragment) {
   "use strict";

   return MainController.extend("custom.MyNewMain", {

      onWebsocketClosed : function() {
         var elem = this.byId("centerTitle");
         elem.setHtmlText("<strong style=\"color: red;\">Client Disconnected !</strong>");
      },

      onInit: function() {
         console.log('MAIN CONTROLLER INIT 2');
         MainController.prototype.onInit.apply(this, arguments);
         this.mgr.handle.setReceiver(this);
         //this.mgr.
         console.log("register my controller for init");
         
         var elem = this.byId("centerTitle");
         let title = "<strong>" + elem.getProperty("htmlText")+ "</strong>";

let pthis = this;
         this.mgr.UT_refresh_invmass_dialog = function () {
            pthis.invMassDialogRefresh();
         }
          
         elem.setHtmlText(title);
      },

      onEveManagerInit: function() {

         MainController.prototype.onEveManagerInit.apply(this, arguments);
         var world = this.mgr.childs[0].childs;

         // this is a prediction that the fireworks GUI is the last element after scenes
         // could loop all the elements in top level and check for typename
         var last = world.length -1;

         if (world[last]._typename == "EventManager") {
            this.fw2gui = (world[last]);
            this.showEventInfo();
         }
      },

      onWebsocketMsg : function(handle, msg, offset)
      {
         this.mgr.onWebsocketMsg(handle, msg, offset);
      },


      sceneElementChange: function(msg) {
         console.log("ddddddxxxxx sceneElementChange", msg)
      },

      showHelp : function(oEvent) {
         alert("=====User support: dummy@cern.ch");
      },

      eventFilterShow: function () {
         if (this.eventFilter){
            this.eventFilter.openFilterDialog();
         }
         else {
            let pthis = this;
            XMLView.create({
               viewName: "custom.view.EventFilter",
            }).then(function (oView) {
               pthis.eventFilter = oView.getController();
               pthis.eventFilter.setGUIElement(pthis.fw2gui);
              // console.log(oView, "filter dialog", oView.byId("filterDialog"));
               pthis.eventFilter.makeTables();
               pthis.eventFilter.openFilterDialog();
            });
         }
      },
      showEventInfo : function() {
         // console.log("showEventInfo");
         let ei = this.fw2gui.fTitle.split("/");
         var event = ei[0];
         var nevents = ei[2];
         var run = ei[2];
         var lumi = ei[3];
         let eiei = ei[4];
         document.title = this.fw2gui.fName +": " + event + "/" + nevents;

         this.byId("runInput").setValue(run);
         this.byId("lumiInput").setValue(lumi);
         this.byId("eventInput").setValue(eiei);

         this.byId("fileName").setText(this.fw2gui.fName);
         this.byId("fileName").setDesign("Bold");


         this.byId("fileNav").setText(event + "/" + nevents);
         this.byId("fileNav").setDesign("Bold");
      },

      nextEvent : function(oEvent) {
          this.mgr.SendMIR("NextEvent()", this.fw2gui.fElementId, "EventManager");
      },

      prevEvent : function(oEvent) {
         this.mgr.SendMIR("PreviousEvent()", this.fw2gui.fElementId, "EventManager");
      },

      toggleGedEditor: function() {
         this.byId("Summary").getController().toggleEditor();
      },

      onPressInvMass: function(oEvent)
      {
			var oButton = oEvent.getSource(),
			oView = this.getView();
            let pthis = this;
			// create popover
			if (!this._pPopover) {
				this._pPopover = Fragment.load({
					id: oView.getId(),
					name: "custom.view.InvMassPopover",
					controller: this
            }).then(function (oPopover) {
               oView.addDependent(oPopover);
					return oPopover;
				});
			}
			this._pPopover.then(function(oPopover) {
            pthis.fw2gui.childs[0].w = oPopover;

            let cl = oPopover.getContent();
            cl[0].setHtmlText("<pre>Press \'Calculate\' button to get result \nof current selection state</pre>");
				oPopover.openBy(oButton);
			});
      },
      handleInvMassCalcPress : function()
      {
		//	this.byId("myPopover").close();

         let inmd =  this.fw2gui.childs[0];
         this.mgr.SendMIR("Calculate()", inmd.fElementId, "InvMassDialog");
      },

      invMassDialogRefresh : function()
      { 
         let inmd = this.fw2gui.childs[0];
         if (inmd.w) {
         let cl = inmd.w.getContent();
         cl[0].setHtmlText(inmd.fTitle);
         }
      },

      autoplay: function (oEvent) {
         console.log("AUTO", oEvent.getParameter("selected"));
         this.mgr.SendMIR("autoplay(" + oEvent.getParameter("selected") + ")", this.fw2gui.fElementId, "EventManager");
      },

      playdelay: function (oEvent) {
         console.log("playdelay ", oEvent.getParameters());
         this.mgr.SendMIR("playdelay(" + oEvent.getParameter("value") + ")", this.fw2gui.fElementId, "EventManager");
      },

   });
});
