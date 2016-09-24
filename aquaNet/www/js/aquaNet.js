$(document).ready(function() {
  $('body').append($('<div class="container-fluid" id="container"><h1>Aquanet</h1><div id="main"><div id="module-list" class="row"></div></div></div>')) ;

  var AquaNetModule = Backbone.Model.extend({
    defaults: function() {
      var model = {
        name: "",     // Name of the module
        type: "master", // master or station
        localIP: "",  // IP on the local domestic wifi network
        APName: "",   // Name of the created wifi network
        APIP: "",     // IP on the Access Point created wifi network
        temp: 0,
        tempAlert: "no",
        minTemp: 0,
        maxTemp: 0,
        light: 0,
        lightAlert: "no",
        minLight: 0,
        maxLight: 0,
        waterLevel: "high",
        power: "on",         // off, on
        oneAlert: "no"
      };
      return model;
    },

    // Server returns some numeric values, not suited to use as class names
    parse: function(data) {

      data.power = data.powerAlert ? "off" : "on";
      data.waterLevel = data.waterLevelAlert ? "low": "high" ;
      data.lightAlert = data.lightAlert ? "isalert" : "noalert";
      data.tempAlert = data.tempAlert ? "isalert" : "noalert";
      data.type = data.type ? "master" : "station";
      data.name = data.name || "Aquarium";
      data.temp = data.temp/100;
      data.minTemp = data.minTemp/100;
      data.maxTemp = data.maxTemp/100;
      return data;
    }
  });

  var url = document.location.href.split('/');
  url.pop();
  url = url.join('/') + "/getData.json"
  var AquaNetList = Backbone.Collection.extend({
    model: AquaNetModule,
    url: url
  });

  var modules = new AquaNetList;

  var AquaNetView = Backbone.View.extend({
    tagName: "div",
    template: _.template('\
<div class="module <%- type %> <%- oneAlert %>">\
<div class="name <%- name %>"><%- name %></div>\
<div class="localIP"><%- localIP %></div>\
<div class="APName"><%- APName %></div>\
<div class="APIP"><span><%- APName %></span><%- APIP %></div>\
<div class="temperature <%- tempAlert %>"><%- temp %></div>\
<div class="temperatureRange"><%- minTemp %> - <%- maxTemp %></div>\
<div class="light <%- lightAlert %>"><%- light %></div>\
<div class="lightRange"><%- minLight %> - <%- maxLight %></div>\
<div class="waterLevel <%- waterLevel %>"></div>\
<div class="power <%- power %>"></div>\
</div>'),
    initialize: function() {
      this.listenTo(this.model, 'change', this.render);
    },

    render: function() {
      this.$el.html(this.template(this.model.toJSON()));
      this.$el.addClass("col-xs-12 col-sm-4 col-md-4 col-lg-3") ;
      return this;
    }
  });

  var AppView = Backbone.View.extend({
    el: $("#main"),
    initialize: function() {
      this.listenTo(modules, 'add', this.addOne);
      this.listenTo(modules, 'reset', this.addAll);
      this.listenTo(modules, 'all', this.render);

    },
    addOne: function(aModule) {
      var view = new AquaNetView({model: aModule});
      this.$("#module-list").append(view.render().el);
    },
    addAll: function() {
      modules.each(this.addOne, this);
    }
  });

  var app = new AppView;
  modules.fetch({
    reset: true,
    error: function(collection, response, options) {
      debugger;
    }
  });

});

