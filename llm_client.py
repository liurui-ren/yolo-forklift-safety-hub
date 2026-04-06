"""
LLM client helpers for image description.
"""

import base64
import mimetypes

from openai import OpenAI

from config import LLM_MODEL, OPENAI_API_KEY, LLM_TIMEOUT_SEC


def _guess_mime_type(image_path):
    mime, _ = mimetypes.guess_type(image_path)
    return mime or "image/jpeg"


def describe_image(image_path):
    """
    Call OpenAI multimodal model to describe the image.
    Returns a short Chinese description (2-3 sentences).
    """
    if not OPENAI_API_KEY:
        raise RuntimeError("OPENAI_API_KEY is not set")

    with open(image_path, "rb") as f:
        image_bytes = f.read()

    mime = _guess_mime_type(image_path)
    b64 = base64.b64encode(image_bytes).decode("ascii")
    data_url = f"data:{mime};base64,{b64}"

    client = OpenAI(api_key=OPENAI_API_KEY, timeout=LLM_TIMEOUT_SEC)
    prompt = (
        "请用中文两到三句话客观描述这张报警图片中的场景。"
        "只描述可见内容，不要给出安全建议或推断原因。"
    )
    response = client.responses.create(
        model=LLM_MODEL,
        input=[
            {
                "role": "user",
                "content": [
                    {"type": "input_text", "text": prompt},
                    {"type": "input_image", "image_url": data_url},
                ],
            }
        ],
    )

    text = (response.output_text or "").strip()
    if not text:
        raise RuntimeError("Empty response from LLM")
    return text
