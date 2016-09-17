$(document).ready(function() {
  $('body').append($('<h1>Aquanet</h1>')) ;
  $('body').append($('<div id="main"><ul id="module-list"></ul></div>')) ;

  var AquaNetModule = Backbone.Model.extend({
    defaults: function() {
      var model = {
        name: "",     // Name of the module
        type: "master", // master or station
        localIP: "",  // IP on the local domestic wifi network
        APName: "",   // Name of the created wifi network
        APIP: "",     // IP on the Access Point created wifi network
        temperature: 0,
        temperatureAlert: "no",
        minTemperature: 0,
        maxTemperature: 0,
        light: 0,
        lightAlert: "no",
        minLight: 0,
        maxLight: 0,
        waterLevel: "high",
        power: "on",         // off, on
        globalAlert: "no"
      };
      return model;
    },

    // Server returns some numeric values, not suited to use as class names
    parse: function(data) {
      data.globalAlert = "noalert";
      if(!data.power || !data.waterLevel
          || data.lightAlert || data.temperatureAlert) {
        data.globalAlert = 'isalert';
      }

      data.power = data.power ? "on" : "off";
      data.waterLevel = data.waterLevel ? "high" : "low";
      data.lightAlert = data.lightAlert ? "isalert" : "noalert";
      data.temperatureAlert = data.temperatureAlert ? "isalert" : "noalert";
      data.type = data.type ? "master" : "station";

      return data;
    }
  });

  var AquaNetList = Backbone.Collection.extend({
    model: AquaNetModule,
    url: "testData.json"
  });

  var modules = new AquaNetList;

  var AquaNetView = Backbone.View.extend({
    tagName: "li",
    template: _.template('\
<div class="module <%- type %> <%- globalAlert %> ">\
<div class="name"><%- name %></div>\
<div class="localIP"><%- localIP %></div>\
<div class="APName">Network Name: <%- APName %></div>\
<div class="APIP"><span><%- APName %></span><%- APIP %></div>\
<div class="temperature <%- temperatureAlert %>"><%- temperature %></div>\
</div>'),
    initialize: function() {
      this.listenTo(this.model, 'change', this.render);
    },

    render: function() {
      this.$el.html(this.template(this.model.toJSON()));
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

