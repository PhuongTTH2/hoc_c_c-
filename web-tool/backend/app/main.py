# app/main.py
from fastapi import FastAPI, Depends
from sqlalchemy.orm import Session
from app.core.database import engine, Base, get_db
from app.models import task
import uvicorn

# Tạo bảng trong database
Base.metadata.create_all(bind=engine)

app = FastAPI(title="Web Tool API", version="1.0.0")

@app.get("/")
def root():
    return {"message": "Web Tool API is running", "status": "ok"}

@app.get("/health")
def health_check(db: Session = Depends(get_db)):
    try:
        # Kiểm tra database
        db.execute("SELECT 1")
        db_status = "connected"
    except Exception as e:
        db_status = f"error: {str(e)}"
    
    return {
        "status": "healthy",
        "database": db_status,
        "services": {
            "api": "running"
        }
    }

@app.get("/api/tasks")
def get_tasks(db: Session = Depends(get_db)):
    from app.models.task import Task
    tasks = db.query(Task).all()
    return {"data": tasks}

if __name__ == "__main__":
    uvicorn.run(app, host="0.0.0.0", port=8000)