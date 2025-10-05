async function fetchCSV(){
  const r = await fetch('/api/log.csv'); if(!r.ok) throw new Error(r.status);
  return await r.text();
}

function parseCSV(text){
  const lines = text.trim().split(/\r?\n/); const head = lines.shift().split(',');
  return lines.map(l=>{ const a=l.split(','); const o={}; head.forEach((h,i)=>o[h]=a[i]); return o; });
}

function filterRows(rows){
  const pump = document.querySelector('#fPump').value;
  const evt = document.querySelector('#fEvt').value;
  const fFrom = document.querySelector('#fFrom').value;
  const fTo = document.querySelector('#fTo').value;
  return rows.filter(r=>{
    if(pump && r.pump!==pump) return false;
    if(evt && r.event!==evt) return false;
    if(fFrom && r.datetime.slice(0,10) < fFrom) return false;
    if(fTo && r.datetime.slice(0,10) > fTo) return false;
    return true;
  });
}

function render(rows){
  const tb = document.querySelector('#logTbl tbody'); tb.innerHTML='';
  rows.forEach(r=>{
    const tr = document.createElement('tr');
    tr.innerHTML = `<td>${r.datetime}</td><td>${r.pump}</td><td>${r.requested_ml}</td><td>${r.current_ml_per_sec}</td><td>${r.current_sec}</td><td>${r.event}</td>`;
    tb.appendChild(tr);
  });
}

function toCSV(rows){
  const head=['datetime','pump','requested_ml','current_ml_per_sec','current_sec','event'];
  const body = rows.map(r=> head.map(h=> r[h]).join(',')).join('\n');
  return head.join(',')+'\n'+body+"\n";
}

async function load(){
  const all = parseCSV(await fetchCSV());
  const fil = filterRows(all);
  render(fil);
}

window.addEventListener('DOMContentLoaded', ()=>{
  document.querySelector('#btnLoad').onclick = load;
  document.querySelector('#btnCSV').onclick = async()=>{
    const csv = toCSV(filterRows(parseCSV(await fetchCSV())));
    const blob = new Blob([csv],{type:'text/csv'});
    const a = Object.assign(document.createElement('a'),{href:URL.createObjectURL(blob), download:'doser_log_filtered.csv'});
    a.click(); URL.revokeObjectURL(a.href);
  };
  document.querySelector('#btnToday').onclick = ()=>{
    const d=new Date(); const s=d.toISOString().slice(0,10);
    document.querySelector('#fFrom').value=s; document.querySelector('#fTo').value=s; load();
  };
  document.querySelector('#btn24h').onclick = ()=>{
    const d=new Date(); const d2=new Date(Date.now()-24*3600*1000);
    document.querySelector('#fFrom').value=d2.toISOString().slice(0,10);
    document.querySelector('#fTo').value=d.toISOString().slice(0,10); load();
  };
  document.querySelector('#btnClear').onclick = async ()=>{
    if(confirm('Clear entire log?')){ await fetch('/api/log_clear',{method:'POST'}); load(); }
  };
  load();
});
