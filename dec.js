function d(msg){
  for(i=0;i<msg.length;i+=2){
    document.write( String.fromCharCode((msg.charCodeAt(i)-0x62)+23*(msg.charCodeAt(i+1)-0x44)) );
  }
}
