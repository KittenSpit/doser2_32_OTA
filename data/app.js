async function jget(u){ const r = await fetch(u); if(!r.ok) throw new Error(r.status); return r.json(); }
async function jpost(u,obj){ const r = await fetch(u,{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(obj)}); if(!r.ok) throw new Error(r.status); return r.text(); }

function connectWS(){
  const ws = new WebSocket(`ws://${location.host}/ws`);
  ws.onmessage = (ev)=>{
    try{
      const j=JSON.parse(ev.data);
      document.querySelector('#status').textContent = JSON.stringify(j,null,2);
    }catch(e){}
  };
  ws.onclose = ()=> setTimeout(connectWS, 2000);
}
connectWS();

window.addEventListener('DOMContentLoaded', async ()=>{
  const $ = s=>document.querySelector(s);
  try{
    const s = await jget('/api/settings');
    $('#p0mlps').value = s.p0.mlps; $('#p0pwm').value = s.p0.pwm;
    $('#p1mlps').value = s.p1.mlps; $('#p1pwm').value = s.p1.pwm;
  }catch(e){}

  $('#btnRun').onclick = async ()=>{
    const pump = +$('#qdPump').value; const ml = +$('#qdML').value;
    await jpost('/api/run',{pump, ml}); alert('Started');
  };
  $('#btnStop').onclick = async ()=>{ await jpost('/api/stop',{}); };

  const sec = ()=> +document.querySelector('#ppSec').value;
  $('#btnPrime0').onclick = ()=> jpost('/api/prime',{pump:0,sec:sec()});
  $('#btnPurge0').onclick = ()=> jpost('/api/purge',{pump:0,sec:sec()});
  $('#btnPrime1').onclick = ()=> jpost('/api/prime',{pump:1,sec:sec()});
  $('#btnPurge1').onclick = ()=> jpost('/api/purge',{pump:1,sec:sec()});

  $('#btnSave').onclick = async()=>{
    const p0={mlps:+$('#p0mlps').value,pwm:+$('#p0pwm').value};
    const p1={mlps:+$('#p1mlps').value,pwm:+$('#p1pwm').value};
    await jpost('/api/settings',{p0,p1}); alert('Saved');
  };

  // Schedule table
  const tbl = document.querySelector('#schedTbl tbody');
  const dayChk = (mask)=>{
    const days=['Su','Mo','Tu','We','Th','Fr','Sa'];
    return `<span class="days" data-mask="${mask}">`+days.map((d,i)=>{
      const on = (mask>>i)&1; return `<label><input type="checkbox" ${on?'checked':''} data-i="${i}">${d}</label>`;
    }).join('')+`</span>`;
  };

  function row(s){
    const time = `${String(s.hour).padStart(2,'0')}:${String(s.minute).padStart(2,'0')}`;
    const tr = document.createElement('tr');
    tr.innerHTML = `
      <td><select class="p">${[0,1].map(x=>`<option ${x==s.pump?'selected':''}>${x}</option>`).join('')}</select></td>
      <td>${dayChk(s.days)}</td>
      <td><input type="time" class="t" value="${time}"></td>
      <td><input type="number" class="ml" step="0.1" min="0" value="${s.ml}"></td>
      <td><button class="del">✕</button></td>`;
    tr.querySelector('.del').onclick = ()=> tr.remove();
    tr.querySelectorAll('.days input').forEach(ch=>{
      ch.onchange = ()=>{
        let mask = 0; tr.querySelectorAll('.days input').forEach((c,i)=>{ if(c.checked) mask |= (1<<i); });
        tr.querySelector('.days').dataset.mask = mask;
      };
    });
    return tr;
  }

  async function loadSched(){
    tbl.innerHTML=''; const arr = await jget('/api/schedule');
    arr.forEach(s=> tbl.appendChild(row(s)));
  }
  loadSched();

  document.querySelector('#addRow').onclick = ()=>{
    tbl.appendChild(row({pump:0,days:0,hour:8,minute:0,ml:5}));
  };
  document.querySelector('#saveSched').onclick = async()=>{
    const out=[];
    tbl.querySelectorAll('tr').forEach(tr=>{
      const pump=+tr.querySelector('.p').value;
      const time=tr.querySelector('.t').value.split(':');
      const hour=+time[0], minute=+time[1];
      const ml=+tr.querySelector('.ml').value;
      const days=+tr.querySelector('.days').dataset.mask;
      out.push({pump,days,hour,minute,ml});
    });
    await jpost('/api/schedule', out);
    alert('Schedule saved');
  };
});
