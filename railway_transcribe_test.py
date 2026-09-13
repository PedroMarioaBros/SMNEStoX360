import base64, glob, os, sys
from faster_whisper import WhisperModel

# Reconstruct the short private test audio from text chunks committed only on the temp branch.
parts = []
for p in sorted(glob.glob('temp_whatsapp_audio/chunk_*.b64')):
    with open(p, 'r', encoding='ascii') as f:
        parts.append(f.read().strip())
audio_bytes = base64.b64decode(''.join(parts))
with open('/tmp/whatsapp_test.ogg', 'wb') as f:
    f.write(audio_bytes)

print(f'AUDIO_BYTES={len(audio_bytes)}', flush=True)
model = WhisperModel('small', device='cpu', compute_type='int8', cpu_threads=2)
segments, info = model.transcribe(
    '/tmp/whatsapp_test.ogg',
    language='pt',
    task='transcribe',
    beam_size=5,
    vad_filter=True,
    word_timestamps=False,
    condition_on_previous_text=True,
)
print(f'LANGUAGE={info.language} PROB={info.language_probability:.4f} DURATION={info.duration:.3f}', flush=True)
print('===TRANSCRIPT_START===', flush=True)
for seg in segments:
    print(f'[{seg.start:.3f} --> {seg.end:.3f}] {seg.text.strip()}', flush=True)
print('===TRANSCRIPT_END===', flush=True)
