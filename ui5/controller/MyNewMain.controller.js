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

			// create popover
			if (!this._pPopover) {
				this._pPopover = Fragment.load({
					id: oView.getId(),
					name: "custom.view.Popover",
					controller: this
            }).then(function (oPopover) {
               oView.addDependent(oPopover);


               var list = new sap.m.List({
                  inset: true
               });

               var data = {
                  navigation: [{ title: "ffffff" }, { title: "fffffffff" }]
               };


               var itemTemplate = new sap.m.StandardListItem({
                  title: "{title}"
               });

               var oModel = new sap.ui.model.json.JSONModel();
               // set the data for the model
               oModel.setData(data);
               // set the model to the list
               list.setModel(oModel);

               // bind Aggregation
               list.bindAggregation("items", "/navigation", itemTemplate);

               oPopover.addContent(list);
					// oPopover.bindElement("/ProductCollection/0");
					return oPopover;
				});
			}
			this._pPopover.then(function(oPopover) {
				oPopover.openBy(oButton);
			});
      },
      handleInvMassCalcPress : function()
      {
			this.byId("myPopover").close();
			alert("Calculate mir has been sent");

      }

   });
});
