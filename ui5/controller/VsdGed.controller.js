sap.ui.define(['rootui5/eve7/controller/Ged.controller',
"sap/ui/core/mvc/XMLView"
], function(GedController, XMLView) {
   "use strict";

   return GedController.extend("custom.VsdGed", {

      onInit: function() {
         GedController.prototype.onInit.apply(this, arguments);

         let oModel = new sap.ui.model.json.JSONModel();
         this.getView().setModel(oModel, "edm")
        },


     // event item
     buildFWDataCollectionSetter : function (el) {
         this.buildREveDataCollectionSetter(el);
         this.oModel.setProperty("/title", this.editorElement.fName + " Editor");
         if (el.var) {
         el.var.forEach((par) => {
             let fname = "makePBC" + par.type + "Setter";
             this[fname](par);
         });
         }
     },

    // bool setter
   makePBCBoolSetter :  function(par)
    {
        let gedFrame = this.getView().byId("GED");
        let gcm = this;
        let widget = new sap.m.CheckBox({
        selected: par.val,
            
        select: function (oEvent) {
            console.log("Bool setter select event", oEvent.getSource());
            let funcName = "UpdatePBParameter";
            let value = oEvent.getSource().getSelected();
            let mir = funcName + "( \"" + oEvent.getSource().desc + "\",\"" + value + "\" )";
            gcm.mgr.SendMIR(mir, gcm.editorElement.fElementId, gcm.editorElement._typename);
        }
        });

        widget.desc = par.name;
            
        let label = new sap.m.Text({ text: par.name });
        label.addStyleClass("sapUiTinyMargin");
            
        let frame = new sap.ui.layout.HorizontalLayout({
        content: [widget, label]
        });
            
        gedFrame.addContent(frame);
    },

    makePBCLongSetter : function(par)
    {
        let gedFrame = this.getView().byId("GED");
        let gcm = this;
        let widget = new sap.m.StepInput({
        min: par.min,
        max: par.max,
        value: par.val,
        change: function (oEvent) {
            console.log("Long setter select event", oEvent.getSource());
            let funcName = "UpdatePBParameter";
            let mir = funcName + "( \"" + oEvent.getSource().desc + "\",\"" + oEvent.getParameter("value") + "\" )";
            gcm.mgr.SendMIR(mir, gcm.editorElement.fElementId, gcm.editorElement._typename);
        }
        });

     widget.desc = par.name;
        
     let label = new sap.m.Text({ text: par.name });
     label.addStyleClass("sapUiTinyMargin");
        
     let frame = new sap.ui.layout.HorizontalLayout({
     content: [widget, label]
     });
        
     gedFrame.addContent(frame);
    },
 
    makePBCDoubleSetter : function(par)
    {  
     let gcm = this;
     let widget = new sap.m.Input({
     value: par.val,
     tooltip: "limit value [ " + par.min + ", " + par.max + "]",
     change: function (oEvent) {
         console.log("Long setter select event", oEvent.getSource());
         
         // validation
         let ival = oEvent.getParameter("value");
         
         if (ival > par.max || ival < par.min) {
             let iControl = oEvent.getSource();
             iControl.setValueState(sap.ui.core.ValueState.Error);
         }
         else {
             let funcName = "UpdatePBParameter";
             let mir = funcName + "( \"" + oEvent.getSource().desc + "\",\"" + oEvent.getParameter("value") + "\" )";
             gcm.mgr.SendMIR(mir, gcm.editorElement.fElementId, gcm.editorElement._typename);
         }
     }
     });

     widget.desc = par.name;
        
     let label = new sap.m.Text({ text: par.name });
     label.addStyleClass("sapUiTinyMargin");
        
     let frame = new sap.ui.layout.HorizontalLayout({
     content: [widget, label]
     });
        
     let gedFrame = this.getView().byId("GED");
     gedFrame.addContent(frame);
    }


    });
}); 
