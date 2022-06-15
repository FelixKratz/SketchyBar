"use strict";(self.webpackChunksketchybar_site=self.webpackChunksketchybar_site||[]).push([[550],{3905:function(e,n,t){t.d(n,{Zo:function(){return s},kt:function(){return f}});var a=t(7294);function r(e,n,t){return n in e?Object.defineProperty(e,n,{value:t,enumerable:!0,configurable:!0,writable:!0}):e[n]=t,e}function i(e,n){var t=Object.keys(e);if(Object.getOwnPropertySymbols){var a=Object.getOwnPropertySymbols(e);n&&(a=a.filter((function(n){return Object.getOwnPropertyDescriptor(e,n).enumerable}))),t.push.apply(t,a)}return t}function o(e){for(var n=1;n<arguments.length;n++){var t=null!=arguments[n]?arguments[n]:{};n%2?i(Object(t),!0).forEach((function(n){r(e,n,t[n])})):Object.getOwnPropertyDescriptors?Object.defineProperties(e,Object.getOwnPropertyDescriptors(t)):i(Object(t)).forEach((function(n){Object.defineProperty(e,n,Object.getOwnPropertyDescriptor(t,n))}))}return e}function l(e,n){if(null==e)return{};var t,a,r=function(e,n){if(null==e)return{};var t,a,r={},i=Object.keys(e);for(a=0;a<i.length;a++)t=i[a],n.indexOf(t)>=0||(r[t]=e[t]);return r}(e,n);if(Object.getOwnPropertySymbols){var i=Object.getOwnPropertySymbols(e);for(a=0;a<i.length;a++)t=i[a],n.indexOf(t)>=0||Object.prototype.propertyIsEnumerable.call(e,t)&&(r[t]=e[t])}return r}var p=a.createContext({}),c=function(e){var n=a.useContext(p),t=n;return e&&(t="function"==typeof e?e(n):o(o({},n),e)),t},s=function(e){var n=c(e.components);return a.createElement(p.Provider,{value:n},e.children)},m={inlineCode:"code",wrapper:function(e){var n=e.children;return a.createElement(a.Fragment,{},n)}},u=a.forwardRef((function(e,n){var t=e.components,r=e.mdxType,i=e.originalType,p=e.parentName,s=l(e,["components","mdxType","originalType","parentName"]),u=c(t),f=r,d=u["".concat(p,".").concat(f)]||u[f]||m[f]||i;return t?a.createElement(d,o(o({ref:n},s),{},{components:t})):a.createElement(d,o({ref:n},s))}));function f(e,n){var t=arguments,r=n&&n.mdxType;if("string"==typeof e||r){var i=t.length,o=new Array(i);o[0]=u;var l={};for(var p in n)hasOwnProperty.call(n,p)&&(l[p]=n[p]);l.originalType=e,l.mdxType="string"==typeof e?e:r,o[1]=l;for(var c=2;c<i;c++)o[c]=t[c];return a.createElement.apply(null,o)}return a.createElement.apply(null,t)}u.displayName="MDXCreateElement"},5484:function(e,n,t){t.r(n),t.d(n,{assets:function(){return s},contentTitle:function(){return p},default:function(){return f},frontMatter:function(){return l},metadata:function(){return c},toc:function(){return m}});var a=t(7462),r=t(3366),i=(t(7294),t(3905)),o=["components"],l={id:"animations",title:"Animations",sidebar_position:1},p=void 0,c={unversionedId:"config/animations",id:"config/animations",title:"Animations",description:"Animating the bar",source:"@site/docs/config/Animations.md",sourceDirName:"config",slug:"/config/animations",permalink:"/SketchyBar/config/animations",tags:[],version:"current",sidebarPosition:1,frontMatter:{id:"animations",title:"Animations",sidebar_position:1},sidebar:"docs",previous:{title:"Querying Information",permalink:"/SketchyBar/config/querying"},next:{title:"Type Nomenclature",permalink:"/SketchyBar/config/types"}},s={},m=[{value:"Animating the bar",id:"animating-the-bar",level:2},{value:"Perform multiple animations chained together",id:"perform-multiple-animations-chained-together",level:3}],u={toc:m};function f(e){var n=e.components,t=(0,r.Z)(e,o);return(0,i.kt)("wrapper",(0,a.Z)({},u,t,{components:n,mdxType:"MDXLayout"}),(0,i.kt)("h2",{id:"animating-the-bar"},"Animating the bar"),(0,i.kt)("p",null,"All transitions between ",(0,i.kt)("inlineCode",{parentName:"p"},"<argb_hex>"),", ",(0,i.kt)("inlineCode",{parentName:"p"},"<integer>")," and ",(0,i.kt)("inlineCode",{parentName:"p"},"<positive_integer>"),"\nvalues can be animated, by prepending the animation command in front of any\nregular ",(0,i.kt)("inlineCode",{parentName:"p"},"--set")," or ",(0,i.kt)("inlineCode",{parentName:"p"},"--bar")," command:"),(0,i.kt)("pre",null,(0,i.kt)("code",{parentName:"pre",className:"language-bash"},"sketchybar --animate <curve> <duration> \\\n           --bar <property>=<value> ... <property>=<value> \\\n           --set <name> <property>=<value> ... <property>=<value>\n")),(0,i.kt)("p",null,"where the ",(0,i.kt)("inlineCode",{parentName:"p"},"<curve>")," is any of the animation curves:"),(0,i.kt)("ul",null,(0,i.kt)("li",{parentName:"ul"},(0,i.kt)("inlineCode",{parentName:"li"},"linear"),", ",(0,i.kt)("inlineCode",{parentName:"li"},"quadratic"),", ",(0,i.kt)("inlineCode",{parentName:"li"},"tanh"),", ",(0,i.kt)("inlineCode",{parentName:"li"},"sin"),", ",(0,i.kt)("inlineCode",{parentName:"li"},"exp"))),(0,i.kt)("p",null,"and the ",(0,i.kt)("inlineCode",{parentName:"p"},"<duration>")," is a positive integer quantifying the number of animation\nsteps."),(0,i.kt)("p",null,"The animation system ",(0,i.kt)("em",{parentName:"p"},"always")," animates between all ",(0,i.kt)("em",{parentName:"p"},"current")," values and the\nvalues specified in a configuration command (i.e. ",(0,i.kt)("inlineCode",{parentName:"p"},"--bar")," or ",(0,i.kt)("inlineCode",{parentName:"p"},"--set")," commands)."),(0,i.kt)("h3",{id:"perform-multiple-animations-chained-together"},"Perform multiple animations chained together"),(0,i.kt)("p",null,"If you want to chain two or more animations together, you can do so by simply\nchanging the property multiple times, e.g.:"),(0,i.kt)("pre",null,(0,i.kt)("code",{parentName:"pre",className:"language-bash"},"sketchybar --animate sin 30 --bar y_offset=10 y_offset=0\n")),(0,i.kt)("p",null,"will animate the bar to the first offset and after that to the second offset.\nYou can chain together as main animations as you like and you can change the\nanimation function in between. This is a nice way to create custom animations\nwith key-frames. You can also make other properties wait with their animation\ntill another animation is finished, by simply setting the property that should\nwait to its current value in the first animation."))}f.isMDXComponent=!0}}]);