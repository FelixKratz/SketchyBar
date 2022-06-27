"use strict";(self.webpackChunksketchybar_site=self.webpackChunksketchybar_site||[]).push([[785],{3905:function(e,t,n){n.d(t,{Zo:function(){return m},kt:function(){return c}});var a=n(7294);function r(e,t,n){return t in e?Object.defineProperty(e,t,{value:n,enumerable:!0,configurable:!0,writable:!0}):e[t]=n,e}function i(e,t){var n=Object.keys(e);if(Object.getOwnPropertySymbols){var a=Object.getOwnPropertySymbols(e);t&&(a=a.filter((function(t){return Object.getOwnPropertyDescriptor(e,t).enumerable}))),n.push.apply(n,a)}return n}function l(e){for(var t=1;t<arguments.length;t++){var n=null!=arguments[t]?arguments[t]:{};t%2?i(Object(n),!0).forEach((function(t){r(e,t,n[t])})):Object.getOwnPropertyDescriptors?Object.defineProperties(e,Object.getOwnPropertyDescriptors(n)):i(Object(n)).forEach((function(t){Object.defineProperty(e,t,Object.getOwnPropertyDescriptor(n,t))}))}return e}function p(e,t){if(null==e)return{};var n,a,r=function(e,t){if(null==e)return{};var n,a,r={},i=Object.keys(e);for(a=0;a<i.length;a++)n=i[a],t.indexOf(n)>=0||(r[n]=e[n]);return r}(e,t);if(Object.getOwnPropertySymbols){var i=Object.getOwnPropertySymbols(e);for(a=0;a<i.length;a++)n=i[a],t.indexOf(n)>=0||Object.prototype.propertyIsEnumerable.call(e,n)&&(r[n]=e[n])}return r}var d=a.createContext({}),o=function(e){var t=a.useContext(d),n=t;return e&&(n="function"==typeof e?e(t):l(l({},t),e)),n},m=function(e){var t=o(e.components);return a.createElement(d.Provider,{value:t},e.children)},k={inlineCode:"code",wrapper:function(e){var t=e.children;return a.createElement(a.Fragment,{},t)}},N=a.forwardRef((function(e,t){var n=e.components,r=e.mdxType,i=e.originalType,d=e.parentName,m=p(e,["components","mdxType","originalType","parentName"]),N=o(n),c=r,g=N["".concat(d,".").concat(c)]||N[c]||k[c]||i;return n?a.createElement(g,l(l({ref:t},m),{},{components:n})):a.createElement(g,l({ref:t},m))}));function c(e,t){var n=arguments,r=t&&t.mdxType;if("string"==typeof e||r){var i=n.length,l=new Array(i);l[0]=N;var p={};for(var d in t)hasOwnProperty.call(t,d)&&(p[d]=t[d]);p.originalType=e,p.mdxType="string"==typeof e?e:r,l[1]=p;for(var o=2;o<i;o++)l[o]=n[o];return a.createElement.apply(null,l)}return a.createElement.apply(null,n)}N.displayName="MDXCreateElement"},6327:function(e,t,n){n.r(t),n.d(t,{assets:function(){return m},contentTitle:function(){return d},default:function(){return c},frontMatter:function(){return p},metadata:function(){return o},toc:function(){return k}});var a=n(7462),r=n(3366),i=(n(7294),n(3905)),l=["components"],p={id:"items",title:"Item Properties",sidebar_position:1},d=void 0,o={unversionedId:"config/items",id:"config/items",title:"Item Properties",description:"Items and their properties",source:"@site/docs/config/Items.md",sourceDirName:"config",slug:"/config/items",permalink:"/SketchyBar/config/items",tags:[],version:"current",sidebarPosition:1,frontMatter:{id:"items",title:"Item Properties",sidebar_position:1},sidebar:"docs",previous:{title:"Bar Properties",permalink:"/SketchyBar/config/bar"},next:{title:"Special Components",permalink:"/SketchyBar/config/components"}},m={},k=[{value:"Items and their properties",id:"items-and-their-properties",level:2},{value:"Adding items to SketchyBar",id:"adding-items-to-sketchybar",level:3},{value:"Changing item properties",id:"changing-item-properties",level:3},{value:"Geometry Properties",id:"geometry-properties",level:3},{value:"Icon properties",id:"icon-properties",level:3},{value:"Label properties",id:"label-properties",level:3},{value:"Scripting properties",id:"scripting-properties",level:3},{value:"Text properties",id:"text-properties",level:3},{value:"Background properties",id:"background-properties",level:3},{value:"Image properties",id:"image-properties",level:3},{value:"Shadow properties",id:"shadow-properties",level:3},{value:"Changing the default values for all further items",id:"changing-the-default-values-for-all-further-items",level:3},{value:"Item Reordering",id:"item-reordering",level:3},{value:"Moving Items to specific positions",id:"moving-items-to-specific-positions",level:3},{value:"Item Cloning",id:"item-cloning",level:3},{value:"Renaming Items",id:"renaming-items",level:3},{value:"Removing Items",id:"removing-items",level:3}],N={toc:k};function c(e){var t=e.components,p=(0,r.Z)(e,l);return(0,i.kt)("wrapper",(0,a.Z)({},N,p,{components:t,mdxType:"MDXLayout"}),(0,i.kt)("h2",{id:"items-and-their-properties"},"Items and their properties"),(0,i.kt)("p",null,"Items are the main building blocks of ",(0,i.kt)("em",{parentName:"p"},"SketchyBar")," and can be configured in a number of ways. Items have the following basic structure:"),(0,i.kt)("p",null,(0,i.kt)("img",{alt:"Item Structure",src:n(2975).Z,width:"499",height:"473"})),(0,i.kt)("h3",{id:"adding-items-to-sketchybar"},"Adding items to SketchyBar"),(0,i.kt)("pre",null,(0,i.kt)("code",{parentName:"pre",className:"language-bash"},"sketchybar --add item <name> <position>\n")),(0,i.kt)("p",null,"where the ",(0,i.kt)("inlineCode",{parentName:"p"},"<name>")," should not contain whitespaces (or must be quoted), it is later used to refer to this item in the configuration.\nThe ",(0,i.kt)("inlineCode",{parentName:"p"},"<position>")," is the placement in the bar and can be either ",(0,i.kt)("inlineCode",{parentName:"p"},"left"),", ",(0,i.kt)("inlineCode",{parentName:"p"},"right"),", ",(0,i.kt)("inlineCode",{parentName:"p"},"center")," or ",(0,i.kt)("inlineCode",{parentName:"p"},"q")," (which is left of the notch) and ",(0,i.kt)("inlineCode",{parentName:"p"},"e")," (which is right of the notch).\nThe items will appear in the bar in the order in which they are added, but can be moved later on."),(0,i.kt)("table",null,(0,i.kt)("thead",{parentName:"table"},(0,i.kt)("tr",{parentName:"thead"},(0,i.kt)("th",{parentName:"tr",align:null},(0,i.kt)("inlineCode",{parentName:"th"},"<name>")),(0,i.kt)("th",{parentName:"tr",align:null},(0,i.kt)("inlineCode",{parentName:"th"},"<string>")))),(0,i.kt)("tbody",{parentName:"table"},(0,i.kt)("tr",{parentName:"tbody"},(0,i.kt)("td",{parentName:"tr",align:null},(0,i.kt)("inlineCode",{parentName:"td"},"<position>")),(0,i.kt)("td",{parentName:"tr",align:null},(0,i.kt)("inlineCode",{parentName:"td"},"left"),", ",(0,i.kt)("inlineCode",{parentName:"td"},"right"),", ",(0,i.kt)("inlineCode",{parentName:"td"},"center"),", (",(0,i.kt)("inlineCode",{parentName:"td"},"q"),", ",(0,i.kt)("inlineCode",{parentName:"td"},"e")," ",(0,i.kt)("a",{parentName:"td",href:"https://github.com/FelixKratz/SketchyBar/issues/120"},"#120"),")")))),(0,i.kt)("h3",{id:"changing-item-properties"},"Changing item properties"),(0,i.kt)("pre",null,(0,i.kt)("code",{parentName:"pre",className:"language-bash"},"sketchybar --set <name> <property>=<value> ... <property>=<value>\n")),(0,i.kt)("p",null,"where the ",(0,i.kt)("inlineCode",{parentName:"p"},"<name>")," is used to target the item.\n(The ",(0,i.kt)("inlineCode",{parentName:"p"},"<name>")," can be a regular expression inside of two slashed: ",(0,i.kt)("inlineCode",{parentName:"p"},"/<regex>/"),")"),(0,i.kt)("p",null,"A list of properties available to the ",(0,i.kt)("em",{parentName:"p"},"set")," command is listed below (components might have additional properties, see the respective component section for them):"),(0,i.kt)("h3",{id:"geometry-properties"},"Geometry Properties"),(0,i.kt)("table",null,(0,i.kt)("thead",{parentName:"table"},(0,i.kt)("tr",{parentName:"thead"},(0,i.kt)("th",{parentName:"tr",align:"center"},"<property",">"),(0,i.kt)("th",{parentName:"tr",align:"center"},"<value",">"),(0,i.kt)("th",{parentName:"tr",align:"center"},"default"),(0,i.kt)("th",{parentName:"tr",align:null},"description"))),(0,i.kt)("tbody",{parentName:"table"},(0,i.kt)("tr",{parentName:"tbody"},(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"drawing")),(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"<boolean>")),(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"on")),(0,i.kt)("td",{parentName:"tr",align:null},"If the item should be drawn into the bar")),(0,i.kt)("tr",{parentName:"tbody"},(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"position")),(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"left"),", ",(0,i.kt)("inlineCode",{parentName:"td"},"right"),", ",(0,i.kt)("inlineCode",{parentName:"td"},"center")),(0,i.kt)("td",{parentName:"tr",align:"center"}),(0,i.kt)("td",{parentName:"tr",align:null},"Position of the item in the bar")),(0,i.kt)("tr",{parentName:"tbody"},(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"associated_space")),(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"<positive_integer list>")),(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"0")),(0,i.kt)("td",{parentName:"tr",align:null},"Spaces to show this item on")),(0,i.kt)("tr",{parentName:"tbody"},(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"associated_display")),(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"<positive_integer list>")),(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"0")),(0,i.kt)("td",{parentName:"tr",align:null},"Displays to show this item on")),(0,i.kt)("tr",{parentName:"tbody"},(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"ignore_association")),(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"<boolean>")),(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"off")),(0,i.kt)("td",{parentName:"tr",align:null},"Ignores all space / display associations while on")),(0,i.kt)("tr",{parentName:"tbody"},(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"y_offset")),(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"<integer>")),(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"0")),(0,i.kt)("td",{parentName:"tr",align:null},"Vertical offset applied to the item")),(0,i.kt)("tr",{parentName:"tbody"},(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"width")),(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"<positive_integer>")," or ",(0,i.kt)("inlineCode",{parentName:"td"},"dynamic")),(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"dynamic")),(0,i.kt)("td",{parentName:"tr",align:null},"Makes the ",(0,i.kt)("em",{parentName:"td"},"item")," use a fixed ",(0,i.kt)("em",{parentName:"td"},"width")," given in points")),(0,i.kt)("tr",{parentName:"tbody"},(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"blur_radius")),(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"<positive_integer>")),(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"0")),(0,i.kt)("td",{parentName:"tr",align:null},"The blur radius applied to the background of the item")))),(0,i.kt)("h3",{id:"icon-properties"},"Icon properties"),(0,i.kt)("table",null,(0,i.kt)("thead",{parentName:"table"},(0,i.kt)("tr",{parentName:"thead"},(0,i.kt)("th",{parentName:"tr",align:"center"},"<property",">"),(0,i.kt)("th",{parentName:"tr",align:"center"},"<value",">"),(0,i.kt)("th",{parentName:"tr",align:"center"},"default"),(0,i.kt)("th",{parentName:"tr",align:null},"description"))),(0,i.kt)("tbody",{parentName:"table"},(0,i.kt)("tr",{parentName:"tbody"},(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"icon")),(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"<string>")),(0,i.kt)("td",{parentName:"tr",align:"center"}),(0,i.kt)("td",{parentName:"tr",align:null},"Icon of the item")),(0,i.kt)("tr",{parentName:"tbody"},(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"icon.<text_property>")),(0,i.kt)("td",{parentName:"tr",align:"center"}),(0,i.kt)("td",{parentName:"tr",align:"center"}),(0,i.kt)("td",{parentName:"tr",align:null},"Icons support all ",(0,i.kt)("em",{parentName:"td"},"text")," properties")))),(0,i.kt)("h3",{id:"label-properties"},"Label properties"),(0,i.kt)("table",null,(0,i.kt)("thead",{parentName:"table"},(0,i.kt)("tr",{parentName:"thead"},(0,i.kt)("th",{parentName:"tr",align:"center"},"<property",">"),(0,i.kt)("th",{parentName:"tr",align:"center"},"<value",">"),(0,i.kt)("th",{parentName:"tr",align:"center"},"default"),(0,i.kt)("th",{parentName:"tr",align:null},"description"))),(0,i.kt)("tbody",{parentName:"table"},(0,i.kt)("tr",{parentName:"tbody"},(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"label")),(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"<string>")),(0,i.kt)("td",{parentName:"tr",align:"center"}),(0,i.kt)("td",{parentName:"tr",align:null},"Label of the item")),(0,i.kt)("tr",{parentName:"tbody"},(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"label.<text_property>")),(0,i.kt)("td",{parentName:"tr",align:"center"}),(0,i.kt)("td",{parentName:"tr",align:"center"}),(0,i.kt)("td",{parentName:"tr",align:null},"Labels support all ",(0,i.kt)("em",{parentName:"td"},"text")," properties")))),(0,i.kt)("h3",{id:"scripting-properties"},"Scripting properties"),(0,i.kt)("table",null,(0,i.kt)("thead",{parentName:"table"},(0,i.kt)("tr",{parentName:"thead"},(0,i.kt)("th",{parentName:"tr",align:"center"},"<property",">"),(0,i.kt)("th",{parentName:"tr",align:"center"},"<value",">"),(0,i.kt)("th",{parentName:"tr",align:"center"},"default"),(0,i.kt)("th",{parentName:"tr",align:null},"description"))),(0,i.kt)("tbody",{parentName:"table"},(0,i.kt)("tr",{parentName:"tbody"},(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"script")),(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"<path>"),", ",(0,i.kt)("inlineCode",{parentName:"td"},"<string>")),(0,i.kt)("td",{parentName:"tr",align:"center"}),(0,i.kt)("td",{parentName:"tr",align:null},"Script to run on an ",(0,i.kt)("inlineCode",{parentName:"td"},"event"))),(0,i.kt)("tr",{parentName:"tbody"},(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"click_script")),(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"<path>"),", ",(0,i.kt)("inlineCode",{parentName:"td"},"<string>")),(0,i.kt)("td",{parentName:"tr",align:"center"}),(0,i.kt)("td",{parentName:"tr",align:null},"Script to run on a mouse click (Difference to ",(0,i.kt)("inlineCode",{parentName:"td"},"mouse.clicked")," event: ",(0,i.kt)("a",{parentName:"td",href:"https://github.com/FelixKratz/SketchyBar/discussions/109"},"#109"),")")),(0,i.kt)("tr",{parentName:"tbody"},(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"update_freq")),(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"<positive_integer>")),(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"1")),(0,i.kt)("td",{parentName:"tr",align:null},"Time in seconds between routine script executions")),(0,i.kt)("tr",{parentName:"tbody"},(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"updates")),(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"<boolean>"),", ",(0,i.kt)("inlineCode",{parentName:"td"},"when_shown")),(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"on")),(0,i.kt)("td",{parentName:"tr",align:null},"If and when the item updates e.g. via script execution")))),(0,i.kt)("h3",{id:"text-properties"},"Text properties"),(0,i.kt)("table",null,(0,i.kt)("thead",{parentName:"table"},(0,i.kt)("tr",{parentName:"thead"},(0,i.kt)("th",{parentName:"tr",align:"center"},"<text_property",">"),(0,i.kt)("th",{parentName:"tr",align:"center"},"<value",">"),(0,i.kt)("th",{parentName:"tr",align:"center"},"default"),(0,i.kt)("th",{parentName:"tr",align:null},"description"))),(0,i.kt)("tbody",{parentName:"table"},(0,i.kt)("tr",{parentName:"tbody"},(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"drawing")),(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"<boolean>")),(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"on")),(0,i.kt)("td",{parentName:"tr",align:null},"If the text is rendered")),(0,i.kt)("tr",{parentName:"tbody"},(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"highlight")),(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"<boolean>")),(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"off")),(0,i.kt)("td",{parentName:"tr",align:null},"If the text uses the ",(0,i.kt)("inlineCode",{parentName:"td"},"highlight_color")," or the regular ",(0,i.kt)("inlineCode",{parentName:"td"},"color"))),(0,i.kt)("tr",{parentName:"tbody"},(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"color")),(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"<argb_hex>")),(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"0xffffffff")),(0,i.kt)("td",{parentName:"tr",align:null},"Color used to render the text")),(0,i.kt)("tr",{parentName:"tbody"},(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"highlight_color")),(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"<argb_hex>")),(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"0xff000000")),(0,i.kt)("td",{parentName:"tr",align:null},"Highlight color of the text (e.g. for active space icon")),(0,i.kt)("tr",{parentName:"tbody"},(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"padding_left")),(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"<integer>")),(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"0")),(0,i.kt)("td",{parentName:"tr",align:null},"Padding to the left of the ",(0,i.kt)("inlineCode",{parentName:"td"},"text"))),(0,i.kt)("tr",{parentName:"tbody"},(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"padding_right")),(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"<integer>")),(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"0")),(0,i.kt)("td",{parentName:"tr",align:null},"Padding to the right of the ",(0,i.kt)("inlineCode",{parentName:"td"},"text"))),(0,i.kt)("tr",{parentName:"tbody"},(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"y_offset")),(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"<integer>")),(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"0")),(0,i.kt)("td",{parentName:"tr",align:null},"Vertical offset applied to the ",(0,i.kt)("inlineCode",{parentName:"td"},"text"))),(0,i.kt)("tr",{parentName:"tbody"},(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"font")),(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"<family>:<type>:<size>")),(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"Hack Nerd Font:Bold:14.0")),(0,i.kt)("td",{parentName:"tr",align:null},"The font to be used for the ",(0,i.kt)("inlineCode",{parentName:"td"},"text"))),(0,i.kt)("tr",{parentName:"tbody"},(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"width")),(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"<positive_integer>")," or ",(0,i.kt)("inlineCode",{parentName:"td"},"dynamic")),(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"dynamic")),(0,i.kt)("td",{parentName:"tr",align:null},"Makes the ",(0,i.kt)("inlineCode",{parentName:"td"},"text")," use a fixed ",(0,i.kt)("inlineCode",{parentName:"td"},"width")," given in points")),(0,i.kt)("tr",{parentName:"tbody"},(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"align")),(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"center"),", ",(0,i.kt)("inlineCode",{parentName:"td"},"left"),", ",(0,i.kt)("inlineCode",{parentName:"td"},"right")),(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"left")),(0,i.kt)("td",{parentName:"tr",align:null},"Aligns the ",(0,i.kt)("inlineCode",{parentName:"td"},"text")," in its container when it has a fixed ",(0,i.kt)("inlineCode",{parentName:"td"},"width")," larger than the content width")),(0,i.kt)("tr",{parentName:"tbody"},(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"background.<background_property>")),(0,i.kt)("td",{parentName:"tr",align:"center"}),(0,i.kt)("td",{parentName:"tr",align:"center"}),(0,i.kt)("td",{parentName:"tr",align:null},"Texts support all ",(0,i.kt)("inlineCode",{parentName:"td"},"background")," properties")),(0,i.kt)("tr",{parentName:"tbody"},(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"shadow.<shadow_property>")),(0,i.kt)("td",{parentName:"tr",align:"center"}),(0,i.kt)("td",{parentName:"tr",align:"center"}),(0,i.kt)("td",{parentName:"tr",align:null},"Texts support all ",(0,i.kt)("inlineCode",{parentName:"td"},"shadow")," properties")))),(0,i.kt)("h3",{id:"background-properties"},"Background properties"),(0,i.kt)("table",null,(0,i.kt)("thead",{parentName:"table"},(0,i.kt)("tr",{parentName:"thead"},(0,i.kt)("th",{parentName:"tr",align:"center"},"<background_property",">"),(0,i.kt)("th",{parentName:"tr",align:"center"},"<value",">"),(0,i.kt)("th",{parentName:"tr",align:"center"},"default"),(0,i.kt)("th",{parentName:"tr",align:null},"description"))),(0,i.kt)("tbody",{parentName:"table"},(0,i.kt)("tr",{parentName:"tbody"},(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"drawing")),(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"<boolean>")),(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"off")),(0,i.kt)("td",{parentName:"tr",align:null},"If the ",(0,i.kt)("inlineCode",{parentName:"td"},"background")," should be rendered")),(0,i.kt)("tr",{parentName:"tbody"},(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"color")),(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"<argb_hex>")),(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"0x00000000")),(0,i.kt)("td",{parentName:"tr",align:null},"Fill color of the ",(0,i.kt)("inlineCode",{parentName:"td"},"background"))),(0,i.kt)("tr",{parentName:"tbody"},(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"border_color")),(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"<argb_hex>")),(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"0x00000000")),(0,i.kt)("td",{parentName:"tr",align:null},"Color of the backgrounds border")),(0,i.kt)("tr",{parentName:"tbody"},(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"border_width")),(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"<positive_integer>")),(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"0")),(0,i.kt)("td",{parentName:"tr",align:null},"Width of the background border")),(0,i.kt)("tr",{parentName:"tbody"},(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"height")),(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"<positive_integer>")),(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"0")),(0,i.kt)("td",{parentName:"tr",align:null},"Overrides the ",(0,i.kt)("inlineCode",{parentName:"td"},"height")," of the background")),(0,i.kt)("tr",{parentName:"tbody"},(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"corner_radius")),(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"<positive_integer>")),(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"0")),(0,i.kt)("td",{parentName:"tr",align:null},"Corner radius of the background")),(0,i.kt)("tr",{parentName:"tbody"},(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"padding_left")),(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"<integer>")),(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"0")),(0,i.kt)("td",{parentName:"tr",align:null},"Padding to the left of the ",(0,i.kt)("inlineCode",{parentName:"td"},"background"))),(0,i.kt)("tr",{parentName:"tbody"},(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"padding_right")),(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"<integer>")),(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"0")),(0,i.kt)("td",{parentName:"tr",align:null},"Padding to the right of the ",(0,i.kt)("inlineCode",{parentName:"td"},"background"))),(0,i.kt)("tr",{parentName:"tbody"},(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"y_offset")),(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"<integer>")),(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"0")),(0,i.kt)("td",{parentName:"tr",align:null},"Vertical offset applied to the ",(0,i.kt)("inlineCode",{parentName:"td"},"background"))),(0,i.kt)("tr",{parentName:"tbody"},(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"image")),(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"<path>"),", ",(0,i.kt)("inlineCode",{parentName:"td"},"app.<bundle-id>")),(0,i.kt)("td",{parentName:"tr",align:"center"}),(0,i.kt)("td",{parentName:"tr",align:null},"The path to a png or jpeg image file, or a bundle identifier of an application")),(0,i.kt)("tr",{parentName:"tbody"},(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"image.<image_property>")),(0,i.kt)("td",{parentName:"tr",align:"center"}),(0,i.kt)("td",{parentName:"tr",align:"center"}),(0,i.kt)("td",{parentName:"tr",align:null},"Backgrounds support all ",(0,i.kt)("inlineCode",{parentName:"td"},"image")," properties")),(0,i.kt)("tr",{parentName:"tbody"},(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"shadow.<shadow_property>")),(0,i.kt)("td",{parentName:"tr",align:"center"}),(0,i.kt)("td",{parentName:"tr",align:"center"}),(0,i.kt)("td",{parentName:"tr",align:null},"Backgrounds support all ",(0,i.kt)("inlineCode",{parentName:"td"},"shadow")," properties")))),(0,i.kt)("h3",{id:"image-properties"},"Image properties"),(0,i.kt)("table",null,(0,i.kt)("thead",{parentName:"table"},(0,i.kt)("tr",{parentName:"thead"},(0,i.kt)("th",{parentName:"tr",align:"center"},"<image_property",">"),(0,i.kt)("th",{parentName:"tr",align:"center"},"<value",">"),(0,i.kt)("th",{parentName:"tr",align:"center"},"default"),(0,i.kt)("th",{parentName:"tr",align:null},"description"))),(0,i.kt)("tbody",{parentName:"table"},(0,i.kt)("tr",{parentName:"tbody"},(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"drawing")),(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"<boolean>")),(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"off")),(0,i.kt)("td",{parentName:"tr",align:null},"If the image should draw")),(0,i.kt)("tr",{parentName:"tbody"},(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"scale")),(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"<positive_integer>")),(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"0")),(0,i.kt)("td",{parentName:"tr",align:null},"The scale factor that should be applied to the image")))),(0,i.kt)("h3",{id:"shadow-properties"},"Shadow properties"),(0,i.kt)("table",null,(0,i.kt)("thead",{parentName:"table"},(0,i.kt)("tr",{parentName:"thead"},(0,i.kt)("th",{parentName:"tr",align:"center"},"<shadow_property",">"),(0,i.kt)("th",{parentName:"tr",align:"center"},"<value",">"),(0,i.kt)("th",{parentName:"tr",align:"center"},"default"),(0,i.kt)("th",{parentName:"tr",align:null},"description"))),(0,i.kt)("tbody",{parentName:"table"},(0,i.kt)("tr",{parentName:"tbody"},(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"drawing")),(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"<boolean>")),(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"off")),(0,i.kt)("td",{parentName:"tr",align:null},"If the shadow should be drawn")),(0,i.kt)("tr",{parentName:"tbody"},(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"color")),(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"<argb_hex>")),(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"0xff000000")),(0,i.kt)("td",{parentName:"tr",align:null},"Color of the shadow")),(0,i.kt)("tr",{parentName:"tbody"},(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"angle")),(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"<positive_integer>")),(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"30")),(0,i.kt)("td",{parentName:"tr",align:null},"Angle of the shadow")),(0,i.kt)("tr",{parentName:"tbody"},(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"distance")),(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"<positive_integer>")),(0,i.kt)("td",{parentName:"tr",align:"center"},(0,i.kt)("inlineCode",{parentName:"td"},"5")),(0,i.kt)("td",{parentName:"tr",align:null},"Distance of the shadow")))),(0,i.kt)("h3",{id:"changing-the-default-values-for-all-further-items"},"Changing the default values for all further items"),(0,i.kt)("p",null,"It is possible to change the ",(0,i.kt)("em",{parentName:"p"},"defaults")," at every point in the configuration. All item created ",(0,i.kt)("em",{parentName:"p"},"after")," changing the defaults will\ninherit these properties from the default item."),(0,i.kt)("pre",null,(0,i.kt)("code",{parentName:"pre",className:"language-bash"},"sketchybar --default <property>=<value> ... <property>=<value>\n")),(0,i.kt)("p",null,"this works for all item properties."),(0,i.kt)("h3",{id:"item-reordering"},"Item Reordering"),(0,i.kt)("p",null,"It is possible to reorder items by invoking"),(0,i.kt)("pre",null,(0,i.kt)("code",{parentName:"pre",className:"language-bash"},"sketchybar --reorder <name> ... <name>\n")),(0,i.kt)("p",null,"where a new order can be supplied for arbitrary items. Only the specified items get reordered, by swapping them around, everything else stays the same. E.g. if you want to swap two items\nsimply call"),(0,i.kt)("pre",null,(0,i.kt)("code",{parentName:"pre",className:"language-bash"},"sketchybar --reorder <item 1> <item 2>\n")),(0,i.kt)("h3",{id:"moving-items-to-specific-positions"},"Moving Items to specific positions"),(0,i.kt)("p",null,"It is possible to move items and order them next to a reference item."),(0,i.kt)("p",null,"Move Item ",(0,i.kt)("inlineCode",{parentName:"p"},"<name>")," to appear ",(0,i.kt)("em",{parentName:"p"},"before")," item ",(0,i.kt)("inlineCode",{parentName:"p"},"<reference name>"),":"),(0,i.kt)("pre",null,(0,i.kt)("code",{parentName:"pre",className:"language-bash"},"sketchybar --move <name> before <reference name>\n")),(0,i.kt)("p",null,"Move Item ",(0,i.kt)("inlineCode",{parentName:"p"},"<name>")," to appear ",(0,i.kt)("em",{parentName:"p"},"after")," item ",(0,i.kt)("inlineCode",{parentName:"p"},"<reference name>"),":"),(0,i.kt)("pre",null,(0,i.kt)("code",{parentName:"pre",className:"language-bash"},"sketchybar --move <name> after <reference name>\n")),(0,i.kt)("h3",{id:"item-cloning"},"Item Cloning"),(0,i.kt)("p",null,"It is possible to clone another item instead of adding a completely blank item"),(0,i.kt)("pre",null,(0,i.kt)("code",{parentName:"pre",className:"language-bash"},"sketchybar --clone <parent name> <name> [optional: before/after]\n")),(0,i.kt)("p",null,"the new item will inherit ",(0,i.kt)("em",{parentName:"p"},"all")," properties of the parent item. The optional ",(0,i.kt)("em",{parentName:"p"},"before")," and ",(0,i.kt)("em",{parentName:"p"},"after")," modifiers can be used\nto move the item ",(0,i.kt)("em",{parentName:"p"},"before"),", or ",(0,i.kt)("em",{parentName:"p"},"after")," the parent, equivalently to a --move command."),(0,i.kt)("h3",{id:"renaming-items"},"Renaming Items"),(0,i.kt)("p",null,"It is possible to rename any item. The new name should obviously not be in use by another item:"),(0,i.kt)("pre",null,(0,i.kt)("code",{parentName:"pre",className:"language-bash"},"sketchybar --rename <old name> <new name>\n")),(0,i.kt)("h3",{id:"removing-items"},"Removing Items"),(0,i.kt)("p",null,"It is possible to remove any item by invoking, the item will be completely destroyed and removed from brackets "),(0,i.kt)("pre",null,(0,i.kt)("code",{parentName:"pre",className:"language-bash"},"sketchybar --remove <name>\n")),(0,i.kt)("p",null,"the ",(0,i.kt)("inlineCode",{parentName:"p"},"<name>")," can again be a regex: ",(0,i.kt)("inlineCode",{parentName:"p"},"/<regex>/"),"."))}c.isMDXComponent=!0},2975:function(e,t,n){t.Z=n.p+"assets/images/bar_item-5cca16299fe8addecdacb3808b574cff.svg"}}]);