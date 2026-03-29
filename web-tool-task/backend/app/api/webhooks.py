from fastapi import APIRouter

router = APIRouter()

@router.post("/webhooks")
def handle_webhook():
    return {"message": "Webhook received"}
