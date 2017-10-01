module.exports = [
  {
    "type": "heading",
    "defaultValue": "Configuration"
  },
     {
        "type": "text",
        "defaultValue": "<h6>A white button = OFF, an orange button = ON</h6>",
      },
	{
    "type": "section",
    "items": [
 	{
        "type": "heading",
        "defaultValue": "<h5>Display</h5>"
	},
	  {
        "type": "toggle",
        "messageKey": "secs",
        "label": "Show BT text or Seconds",
        "defaultValue": false
	  },
		{
        "type": "toggle",
        "messageKey": "invert",
        "label": "Invert",
        "defaultValue": false
      }
	
	]
},
	  {
    "type": "section",
    "items": [	
 	  {
        "type": "heading",
        "defaultValue": "<h5>Vibration</h5>"
      },
      {
        "type": "toggle",
        "messageKey": "bluetoothvibe",
        "label": "Bluetooth Vibration",
        "defaultValue": false
      },
	  {
        "type": "toggle",
        "messageKey": "hourlyvibe",
        "label": "Vibrate each hour",
        "defaultValue": false
      }
    ]
  },
  {
        "type": "text",
        "defaultValue": "<h6>If you find this watchface useful, please consider making a <a href='https://www.paypal.me/markchopsreed'>small donation here</a>. Thankyou.</h6>",
  },
  {
    "type": "submit",
    "defaultValue": "Save Settings"
  }
];