'use strict';

const express = require('express');
const line = require('@line/bot-sdk');
const PORT = process.env.PORT || 3000;

const config = {
    channelSecret: '055ea9689f8182f7977e05445d3f1578',
    channelAccessToken: 'oJwteWmvxYAF+fnDjya45JPcPwYJGl4eYGhLW3TmyFEHt9jWlRERlyOI8Dl5xXpggaWx4n2SAOVgYqOwtaT7iE6gS+ft4KtDDPyl3kqppThlHIUJwT9jQwXlL3TBhpMlOPt/IGpAEZqDCqx1ow/VjgdB04t89/1O/w1cDnyilFU='
};

const app = express();

app.get('/', (req, res) => res.send('Hello LINE BOT!(GET)')); //ブラウザ確認用(無くても問題ない)
app.post('/webhook', line.middleware(config), (req, res) => {
    console.log(req.body.events);

    //ここのif分はdeveloper consoleの"接続確認"用なので削除して問題ないです。
    if(req.body.events[0].replyToken === '00000000000000000000000000000000' && req.body.events[1].replyToken === 'ffffffffffffffffffffffffffffffff'){
        res.send('Hello LINE BOT!(POST)');
        console.log('疎通確認用');
        return;
    }

    Promise
      .all(req.body.events.map(handleEvent))
      .then((result) => res.json(result));
});

const client = new line.Client(config);

async function handleEvent(event) {
  if (event.type === 'message') {
    return client.replyMessage(event.replyToken, {
      type: 'text',
      text: event.message.text //オウム返し
    });
  } else if (event.beacon.type === 'enter') {
    return client.replyMessage(event.replyToken, {
      type: 'text',
      text: '石井の部屋へようこそ！どうぞごゆっくり！ \n https://www.drifting-clouds.com/' //実際に返信の言葉を入れる箇所
    });
  // } else if (event.beacon.type === 'stay') {
  //   return client.replyMessage(event.replyToken, {
  //     type: 'text',
  //     text: 'ご飯食べてく？それともお風呂？' //実際に返信の言葉を入れる箇所
  //   });
  } else {
    return Promise.resolve(null);
  }
}

app.listen(PORT);
console.log(`Server running at ${PORT}`);
